class Application {
    configure_opengl(gl) {
        gl.clearColor(0, 0, 0, 1)
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
        gl.disable(gl.CULL_FACE)
        gl.disable(gl.BLEND)
        gl.disable(gl.DEPTH_TEST)
    }
    load_programs(g, gl) {
        g.make_program(gl, 'texture')
    }
    load_images(g, gl) {
        g.make_image(gl, 'map', gl.CLAMP_TO_EDGE)
        g.make_image(gl, 'you', gl.CLAMP_TO_EDGE)
        g.make_image(gl, 'skeleton', gl.CLAMP_TO_EDGE)
    }
    resize() {
        let gl = this.gl
        let canvas = this.canvas
        let screen = this.screen

        canvas.width = window.innerWidth
        canvas.height = window.innerHeight

        let canvas_ortho = Matrix.Make()
        let draw_ortho = Matrix.Make()

        let scale = 0.5
        let draw_width = canvas.width * scale
        let draw_height = canvas.height * scale

        Matrix.Orthographic(draw_ortho, 0.0, draw_width, 0.0, draw_height, 0.0, 1.0)
        Matrix.Orthographic(canvas_ortho, 0.0, canvas.width, 0.0, canvas.height, 0.0, 1.0)

        let frame = new FrameBuffer(gl, draw_width, draw_height, [gl.RGB], [gl.RGB], [gl.UNSIGNED_BYTE], false, true)

        screen.zero()
        Render.Image(screen, 0, 0, canvas.width, canvas.height, 0.0, 1.0, 1.0, 0.0)
        RenderSystem.UpdateVao(gl, screen)

        this.frame = frame
        this.canvas_ortho = canvas_ortho
        this.draw_ortho = draw_ortho
    }
    constructor() {

        let self = this

        let canvas = document.createElement('canvas')
        canvas.style.display = 'block'
        canvas.style.position = 'absolute'
        canvas.style.left = '0'
        canvas.style.right = '0'
        canvas.style.top = '0'
        canvas.style.bottom = '0'
        canvas.style.margin = 'auto'
        canvas.width = window.innerWidth
        canvas.height = window.innerHeight

        let gl = canvas.getContext('webgl2')
        let g = new RenderSystem()

        this.configure_opengl(gl)
        this.load_programs(g, gl)
        this.load_images(g, gl)

        let generic = RenderBuffer.Init(gl, 2, 0, 2, 1600, 2400)
        let sprite_buffers = new Map()
        sprite_buffers['you'] = RenderBuffer.Init(gl, 2, 0, 2, 40, 60)
        sprite_buffers['skeleton'] = RenderBuffer.Init(gl, 2, 0, 2, 40, 60)

        let screen = RenderBuffer.Init(gl, 2, 0, 2, 4, 6)

        let s = 16.0
        let w = 1.0 / 256.0
        let h = 1.0 / 128.0
        let sprite_cavern = new Map()
        sprite_cavern['dirt'] = new Sprite(1 + 17 * 0, 1 + 17 * 0, s, s, w, h, 0, 0)
        sprite_cavern['dirt light'] = new Sprite(1 + 17 * 0, 1 + 17 * 1, s, s, w, h, 0, 0)
        sprite_cavern['dirt lightest'] = new Sprite(1 + 17 * 0, 1 + 17 * 2, s, s, w, h, 0, 0)
        sprite_cavern['wall'] = new Sprite(1 + 17 * 1, 1 + 17 * 0, s, s, w, h, 0, 0)
        sprite_cavern['wall edge'] = new Sprite(1 + 17 * 1, 1 + 17 * 1, s, s, w, h, 0, 0)
        sprite_cavern['wall corner'] = new Sprite(1 + 17 * 1, 1 + 17 * 2, s, s, w, h, 0, 0)
        sprite_cavern['stone floor'] = new Sprite(1 + 17 * 1, 1 + 17 * 3, s, s, w, h, 0, 0)

        let you_walk = [new Sprite(0, 0, 16, 30, 1.0 / 16.0, 1.0 / 30.0, 0, 0)]
        let skeleton_walk = [new Sprite(0, 0, 16, 31, 1.0 / 16.0, 1.0 / 31.0, 0, 0)]

        let world = new World(8, 3)
        world.build(gl)

        let player = new Thing(world, 'you', you_walk, BLOCK_SIZE * (TILE_SIZE + 9), (BLOCK_SIZE + 1) * TILE_SIZE)
        new Thing(world, 'skeleton', skeleton_walk, BLOCK_SIZE * (TILE_SIZE + 14), (BLOCK_SIZE + 1) * TILE_SIZE)
        new Thing(world, 'skeleton', skeleton_walk, BLOCK_SIZE * (TILE_SIZE + 64), (BLOCK_SIZE + 1) * TILE_SIZE)

        window.onblur = function () {
            self.on = false
        }

        window.onfocus = function () {
            self.on = true
        }

        window.onresize = function () {
            self.resize()
        }

        document.onkeyup = Input.KeyUp
        document.onkeydown = Input.KeyDown
        document.onmouseup = Input.MouseUp
        document.onmousedown = Input.MouseDown
        document.onmousemove = Input.MouseMove

        this.canvas = canvas
        this.screen = screen
        this.player = player
        this.sprite_buffers = sprite_buffers
        this.on = true
        this.gl = gl
        this.g = g
        this.generic = generic
        this.world = world
        this.sprite_cavern = sprite_cavern

        this.resize()
    }
    run() {
        for (let key in this.g.shaders) {
            if (this.g.shaders[key] === null) {
                setTimeout(run, 500)
                return
            }
        }
        for (let key in this.g.textures) {
            if (this.g.textures[key] === null) {
                setTimeout(run, 500)
                return
            }
        }
        document.body.appendChild(this.canvas)
        this.loop()
    }
    loop() {
        if (this.on) {
            this.update()
            this.render()
        }
        requestAnimationFrame(loop)
    }
    update() {
        const alternate = true

        if (alternate) {
            if (Input.Is('ArrowLeft')) this.player.move_left()
            if (Input.Is('ArrowRight')) this.player.move_right()
            if (Input.Is('ArrowDown')) this.player.crouch()
            if (Input.Is('Control')) this.player.block()
            if (Input.Is('a')) this.player.parry()
            if (Input.Is(' ')) this.player.jump()
            if (Input.Is('s')) this.player.dodge()
            if (Input.Is('z')) this.player.light_attack()
            if (Input.Is('x')) this.player.heavy_attack()
            if (Input.Is('Shift')) this.player.sprint(true)
            else this.player.sprint(false)
        } else {
            if (Input.Is('a')) this.player.move_left()
            if (Input.Is('d')) this.player.move_right()
            if (Input.Is('s')) this.player.crouch()
            if (Input.Is('q')) this.player.block()
            if (Input.Is('Control')) this.player.parry()
            if (Input.Is('Shift')) this.player.sprint(true)
            else this.player.sprint(false)
            if (Input.Is(' ')) {
                if (Input.Is('a') || Input.Is('d')) this.player.jump()
                else this.player.dodge()
            }
            if (Input.Is('h')) this.player.light_attack()
            if (Input.Is('u')) this.player.heavy_attack()
        }

        this.world.update()
    }
    render() {
        let g = this.g
        let gl = this.gl
        let frame = this.frame
        let player = this.player

        let view_x = -Math.floor(player.x - frame.width * 0.5)
        let view_y = -Math.floor(player.y - frame.height * 0.5)

        RenderSystem.SetFrameBuffer(gl, frame.fbo)
        RenderSystem.SetView(gl, 0, 0, frame.width, frame.height)
        gl.clear(gl.COLOR_BUFFER_BIT)
        g.set_program(gl, 'texture')
        g.set_orthographic(this.draw_ortho, view_x, view_y)
        g.update_mvp(gl)
        this.world.render(g, gl, frame, player.x, player.y, this.sprite_buffers)

        RenderSystem.SetFrameBuffer(gl, null)
        RenderSystem.SetView(gl, 0, 0, this.canvas.width, this.canvas.height)
        g.set_program(gl, 'texture')
        g.set_orthographic(this.canvas_ortho, 0, 0)
        g.update_mvp(gl)
        g.set_texture_direct(gl, frame.textures[0])
        RenderSystem.BindAndDraw(gl, this.screen)
    }
}

let app = new Application()
app.run()

function run() {
    app.run()
}

function loop() {
    app.loop()
}