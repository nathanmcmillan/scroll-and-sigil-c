package main

import (
	"bytes"
	"encoding/binary"
	"strings"
	"syscall/js"
	"time"

	"./fast"
	"./graphics"
	"./matrix"
	"./render"
)

type worldState struct {
	app            *app
	snapshotTime   int64
	previousUpdate int64
	chatbox        []string
}

func worldStateInit(a *app) *worldState {
	s := &worldState{}
	s.app = a
	return s
}

func (me *worldState) serverUpdates() {
	world := me.app.world
	socketQueue := me.app.socketQueue

	for i := 0; i < len(socketQueue); i++ {
		// dat := bytes.NewReader(socketQueue[i])
		data := fast.ByteReaderInit(socketQueue[i])

		// var uint32ref uint32
		// binary.Read(dat, binary.LittleEndian, &uint32ref)
		// me.snapshotTime = int64(uint32ref) + 1552330000000
		me.snapshotTime = data.GetUint32()
		me.previousUpdate = time.Now().UnixNano()

		// var broadcastCount uint8
		// binary.Read(dat, binary.LittleEndian, &broadcastCount)
		var broadcastCount = data.GetUint8()

		for b := uint8(0); b < broadcastCount; b++ {
			// var broadcastType uint8
			// binary.Read(dat, binary.LittleEndian, &broadcastType)
			broadcastType := data.GetUint8()
			switch broadcastType {
			case BroadcastNew:
				var uid uint16
				var nid uint16
				binary.Read(dat, binary.LittleEndian, &uid)
				binary.Read(dat, binary.LittleEndian, &nid)
				if _, ok := world.netLookup[nid]; ok {
					break
				}
				var x float32
				var y float32
				var z float32
				binary.Read(dat, binary.LittleEndian, &x)
				binary.Read(dat, binary.LittleEndian, &y)
				binary.Read(dat, binary.LittleEndian, &z)
				switch uid {
				case PlasmaUID:
					var dx float32
					var dy float32
					var dz float32
					var damage uint16
					binary.Read(dat, binary.LittleEndian, &dx)
					binary.Read(dat, binary.LittleEndian, &dy)
					binary.Read(dat, binary.LittleEndian, &dz)
					binary.Read(dat, binary.LittleEndian, &damage)
					plasmaInit(world, nid, damage, x, y, z, dx, dy, dz)
				case HumanUID:
					var angle float32
					var health uint16
					var status uint8
					binary.Read(dat, binary.LittleEndian, &angle)
					binary.Read(dat, binary.LittleEndian, &health)
					binary.Read(dat, binary.LittleEndian, &status)
					humanInit(world, nid, x, y, z, angle, health, status)
				default:
					panic("unknown UID")
				}
			case BroadcastDelete:
				var nid uint16
				binary.Read(dat, binary.LittleEndian, &nid)
				if thing, ok := world.netLookup[nid]; ok {
					thing.cleanup()
				}
			case BroadcastChat:
				var size uint8
				binary.Read(dat, binary.LittleEndian, &size)
				chat := &strings.Builder{}
				for ch := uint8(0); ch < size; ch++ {
					var char uint8
					binary.Read(dat, binary.LittleEndian, &char)
					chat.WriteByte(char)
				}
				me.chatbox = append(me.chatbox, chat.String())
			}
		}

		var thingCount uint16
		binary.Read(dat, binary.LittleEndian, &thingCount)
		for t := uint16(0); t < thingCount; t++ {
			var nid uint16
			binary.Read(dat, binary.LittleEndian, &nid)
			thing, ok := world.netLookup[nid]
			if !ok {
				panic("missing thing nid")
			}
			var delta uint8
			binary.Read(dat, binary.LittleEndian, &delta)
			thing.netUpdate(dat, delta)
		}
	}
}

func (me *worldState) update() {
	world := me.app.world
	socketQueue := me.app.socketQueue

	if len(socketQueue) > 0 {
		me.serverUpdates()
		me.app.socketQueue = make([][]byte, 0)
	}

	world.update()

	socketSend := me.app.socketSend
	socketSendOperations := uint8(0)
	size := len(socketSend)
	if size > 0 {
		full := &bytes.Buffer{}
		body := &bytes.Buffer{}
		for op, value := range socketSend {
			binary.Write(body, binary.LittleEndian, op)
			switch op {
			case inputOpNewMove:
				binary.Write(body, binary.LittleEndian, value.(float32))
			case inputOpChat:
				chat := value.(string)
				chatSize := uint8(len(chat))
				if chatSize > 255 {
					chatSize = 255
				}
				binary.Write(body, binary.LittleEndian, chatSize)
				for ch := uint8(0); ch < chatSize; ch++ {
					binary.Write(body, binary.LittleEndian, chat[ch])
				}
			}
			socketSendOperations++
		}
		binary.Write(full, binary.LittleEndian, socketSendOperations)
		binary.Write(full, binary.LittleEndian, body.Bytes())
		me.app.socket.Call("send", js.TypedArrayOf(full.Bytes()))

		for key := range socketSend {
			delete(socketSend, key)
		}
	}
}

func (me *worldState) render() {
	app := me.app
	g := app.g
	gl := app.gl
	frameGeo := app.frameGeo
	frame := app.frame
	canvas := app.canvas
	canvasOrtho := app.canvasOrtho
	drawPerspective := app.drawPerspective
	drawOrtho := app.drawOrtho
	drawImages := app.drawImages
	screen := app.screen
	world := app.world
	cam := app.camera

	cam.update(world)

	graphics.RenderSystemSetFrameBuffer(gl, frameGeo.Fbo)
	graphics.RenderSystemSetView(gl, 0, 0, frame.Width, frame.Height)

	gl.Call("clear", graphics.GLxColorBufferBit)
	gl.Call("clear", graphics.GLxDepthBufferBit)

	g.SetProgram(gl, "copy")
	g.SetOrthographic(drawOrtho, 0, 0)
	g.UpdateMvp(gl)
	g.SetTexture(gl, "sky")
	drawImages.Zero()

	turnX := float32(frame.Width * 2)
	skyX := cam.ry / Tau * turnX
	if skyX >= turnX {
		skyX -= turnX
	}
	frameHeight := float32(frame.Height)
	skyYOffset := cam.rx / Tau * frameHeight
	skTrueHeight := float32(g.Textures["sky"].Height)
	skyHeight := skTrueHeight * 2
	skyTop := frameHeight - skyHeight
	skyY := float32(skyTop)*0.5 + skyYOffset
	if skyY > skyTop {
		skyY = skyTop
	}
	render.Image(drawImages, -skyX, skyY, turnX*2, skyHeight, 0, 0, 2, 1)
	graphics.RenderSystemUpdateAndDraw(gl, drawImages)

	gl.Call("enable", graphics.GLxDepthTest)
	gl.Call("enable", graphics.GLxCullFace)

	g.SetPerspective(drawPerspective, -cam.x, -cam.y, -cam.z, cam.rx, cam.ry)

	camBlockX := int(cam.x * InverseBlockSize)
	camBlockY := int(cam.y * InverseBlockSize)
	camBlockZ := int(cam.z * InverseBlockSize)

	world.render(g, camBlockX, camBlockY, camBlockZ, cam.x, cam.z, cam.ry)

	gl.Call("disable", graphics.GLxCullFace)
	gl.Call("disable", graphics.GLxDepthTest)

	const noShade = 0
	const motionBlur = 1
	const antiAlias = 2
	shading := noShade

	if shading == motionBlur {
		frame2 := app.frame2
		frameScreen := app.frameScreen

		drawInversePerspective := app.drawInversePerspective
		drawInverseMv := app.drawInverseMv
		drawPreviousMvp := app.drawPreviousMvp
		drawCurrentToPreviousMvp := app.drawCurrentToPreviousMvp
		matrix.Inverse(drawInverseMv, g.ModelView)
		matrix.Multiply(drawCurrentToPreviousMvp, drawPreviousMvp, drawInverseMv)
		for i := 0; i < 16; i++ {
			drawPreviousMvp[i] = g.ModelViewProject[i]
		}

		graphics.RenderSystemSetFrameBuffer(gl, frame2.Fbo)
		graphics.RenderSystemSetView(gl, 0, 0, frame2.Width, frame2.Height)
		g.SetProgram(gl, "motion")
		g.SetUniformMatrix4(gl, "inverse_projection", drawInversePerspective)
		g.SetUniformMatrix4(gl, "current_to_previous_matrix", drawCurrentToPreviousMvp)
		g.SetOrthographic(drawOrtho, 0, 0)
		g.UpdateMvp(gl)
		g.SetTextureDirect(gl, frameGeo.Textures[0])
		g.SetIndexTextureDirect(gl, graphics.GLxTexture1, 1, "u_texture1", frameGeo.DepthTexture)
		graphics.RenderSystemBindAndDraw(gl, frameScreen)

		graphics.RenderSystemSetFrameBuffer(gl, js.Null())
		graphics.RenderSystemSetView(gl, 0, 0, canvas.width, canvas.height)
		g.SetProgram(gl, "screen")
		g.SetOrthographic(canvasOrtho, 0, 0)
		g.UpdateMvp(gl)
		g.SetTextureDirect(gl, frame2.Textures[0])
		graphics.RenderSystemBindAndDraw(gl, screen)
	} else if shading == antiAlias {
		graphics.RenderSystemSetFrameBuffer(gl, js.Null())
		graphics.RenderSystemSetView(gl, 0, 0, canvas.width, canvas.height)
		g.SetProgram(gl, "fxaa")
		g.SetUniformVec2(gl, "texel", 1.0/float32(canvas.width), 1.0/float32(canvas.height))
		g.SetOrthographic(canvasOrtho, 0, 0)
		g.UpdateMvp(gl)
		g.SetTextureDirect(gl, frameGeo.Textures[0])
		graphics.RenderSystemBindAndDraw(gl, screen)
	} else {
		g.SetProgram(gl, "texture2d")
		g.SetOrthographic(drawOrtho, 0, 0)
		g.UpdateMvp(gl)
		g.SetTexture(gl, "font")
		drawImages.Zero()
		chatbox := me.chatbox
		y := float32(10)
		for ch := 0; ch < len(chatbox); ch++ {
			render.Print(drawImages, 10, y, chatbox[ch], 2)
			y += render.FontHeight * 2
		}
		graphics.RenderSystemUpdateAndDraw(gl, drawImages)

		graphics.RenderSystemSetFrameBuffer(gl, js.Null())
		graphics.RenderSystemSetView(gl, 0, 0, canvas.width, canvas.height)

		g.SetProgram(gl, "screen")
		g.SetOrthographic(canvasOrtho, 0, 0)
		g.UpdateMvp(gl)
		g.SetTextureDirect(gl, frameGeo.Textures[0])
		graphics.RenderSystemBindAndDraw(gl, screen)
	}
}
