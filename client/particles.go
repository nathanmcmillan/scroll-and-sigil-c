package main

import (
	"math"

	"./graphics"
	"./render"
)

var (
	plasmaExplosionAnimation []*render.Sprite
)

type particle struct {
	world          *world
	sid            string
	sprite         *render.Sprite
	animation      []*render.Sprite
	animationMod   int
	animationFrame int
	x              float32
	y              float32
	z              float32
	deltaX         float32
	deltaY         float32
	deltaZ         float32
	minBX          int
	minBY          int
	minBZ          int
	maxBX          int
	maxBY          int
	maxBZ          int
	radius         float32
	height         float32
	update         func() bool
}

func (me *particle) blockBorders() {
	me.minBX = int((me.x - me.radius) * InverseBlockSize)
	me.minBY = int(me.y * InverseBlockSize)
	me.minBZ = int((me.z - me.radius) * InverseBlockSize)
	me.maxBX = int((me.x + me.radius) * InverseBlockSize)
	me.maxBY = int((me.y + me.height) * InverseBlockSize)
	me.maxBZ = int((me.z + me.radius) * InverseBlockSize)
}

func (me *particle) addToBlocks() {
	for gx := me.minBX; gx <= me.maxBX; gx++ {
		for gy := me.minBY; gy <= me.maxBY; gy++ {
			for gz := me.minBZ; gz <= me.maxBZ; gz++ {
				block := me.world.getBlock(gx, gy, gz)
				block.addParticle(me)
			}
		}
	}
}

func (me *particle) removeFromBlocks() {
	for gx := me.minBX; gx <= me.maxBX; gx++ {
		for gy := me.minBY; gy <= me.maxBY; gy++ {
			for gz := me.minBZ; gz <= me.maxBZ; gz++ {
				block := me.world.getBlock(gx, gy, gz)
				block.removeParticle(me)
			}
		}
	}
}

func (me *particle) updateAnimation() int {
	me.animationFrame++
	if me.animationMod == AnimationRate {
		me.animationMod = 0
		me.animationFrame++
		size := len(me.animation)
		if me.animationFrame == size-1 {
			return AnimationAlmostDone
		} else if me.animationFrame == size {
			return AnimationDone
		}
	}
	return AnimationNotDone
}

func (me *particle) collision() bool {
	minGX := int(me.x - me.radius)
	minGY := int(me.y)
	minGZ := int(me.z - me.radius)
	maxGX := int(me.x + me.radius)
	maxGY := int(me.y + me.height)
	maxGZ := int(me.z + me.radius)
	for gx := minGX; gx <= maxGX; gx++ {
		for gy := minGY; gy <= maxGY; gy++ {
			for gz := minGZ; gz <= maxGZ; gz++ {
				bx := int(float32(gx) * InverseBlockSize)
				by := int(float32(gy) * InverseBlockSize)
				bz := int(float32(gz) * InverseBlockSize)
				tx := gx - bx*BlockSize
				ty := gy - by*BlockSize
				tz := gz - bz*BlockSize
				tile := me.world.getTileType(bx, by, bz, tx, ty, tz)
				if TileClosed[tile] {
					return true
				}
			}
		}
	}
	return false
}

func (me *particle) render(spriteBuffer map[string]*graphics.RenderBuffer, camX, camZ float32) {
	sin := float64(camX - me.x)
	cos := float64(camZ - me.z)
	length := math.Sqrt(sin*sin + cos*cos)
	sin /= length
	cos /= length
	render.RendSprite(spriteBuffer[me.sid], me.x, me.y, me.z, float32(sin), float32(cos), me.sprite)
}

func plasmaExplosionInit(world *world, x, y, z float32) *particle {
	p := &particle{}
	p.world = world
	p.x = x
	p.y = y
	p.z = z
	p.sid = "particles"
	p.animation = plasmaExplosionAnimation
	p.sprite = p.animation[0]
	p.radius = 0.2
	p.height = 0.2
	p.update = p.plasmaUpdate
	world.addParticle(p)
	p.blockBorders()
	p.addToBlocks()
	return p
}

func (me *particle) plasmaUpdate() bool {
	if me.updateAnimation() == AnimationDone {
		me.removeFromBlocks()
		return true
	}
	me.sprite = me.animation[me.animationFrame]
	return false
}

func bloodInit(world *world, x, y, z, dx, dy, dz float32, spriteName string) *particle {
	b := &particle{}
	b.world = world
	b.x = x
	b.y = y
	b.z = z
	b.sid = "particles"
	b.sprite = wadSpriteData[b.sid][spriteName]
	b.deltaX = dx
	b.deltaY = dy
	b.deltaZ = dz
	b.radius = 0.2
	b.height = 0.2
	b.update = b.bloodUpdate
	world.addParticle(b)
	b.blockBorders()
	b.addToBlocks()
	return b
}

func (me *particle) bloodUpdate() bool {
	me.deltaX *= 0.95
	me.deltaY -= 0.01
	me.deltaZ *= 0.95
	me.x += me.deltaX
	me.y += me.deltaY
	me.z += me.deltaZ
	if me.collision() {
		me.removeFromBlocks()
		return true
	}
	me.removeFromBlocks()
	me.blockBorders()
	me.addToBlocks()
	return false
}
