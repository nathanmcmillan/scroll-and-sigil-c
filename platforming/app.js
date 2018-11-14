const MUSIC = new Map()
const SOUND = new Map()
const SPRITES = new Map()

class Application {
    configure_opengl(gl) {
        gl.clearColor(0, 0, 0, 1)
        gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
        gl.disable(gl.CULL_FACE)
        gl.disable(gl.BLEND)
        gl.disable(gl.DEPTH_TEST)
    }
    load_programs(g, gl) {
        g.make_program(gl, "texture")
    }
    load_images(g, gl) {
        g.make_image(gl, "map", gl.CLAMP_TO_EDGE)
        g.make_image(gl, "you", gl.CLAMP_TO_EDGE)
        g.make_image(gl, "skeleton", gl.CLAMP_TO_EDGE)
        g.make_image(gl, "doodad", gl.CLAMP_TO_EDGE)
        g.make_image(gl, "item", gl.CLAMP_TO_EDGE)
        g.make_image(gl, "ui", gl.CLAMP_TO_EDGE)
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

        let canvas = document.createElement("canvas")
        canvas.style.display = "block"
        canvas.style.position = "absolute"
        canvas.style.left = "0"
        canvas.style.right = "0"
        canvas.style.top = "0"
        canvas.style.bottom = "0"
        canvas.style.margin = "auto"
        canvas.width = window.innerWidth
        canvas.height = window.innerHeight

        let gl = canvas.getContext("webgl2")
        let g = new RenderSystem()

        this.configure_opengl(gl)
        this.load_programs(g, gl)
        this.load_images(g, gl)

        let generic = RenderBuffer.Init(gl, 2, 0, 2, 800, 1200)
        let generic2 = RenderBuffer.Init(gl, 2, 0, 2, 800, 1200)
        let sprite_buffers = new Map()
        sprite_buffers["you"] = RenderBuffer.Init(gl, 2, 0, 2, 40, 60)
        sprite_buffers["skeleton"] = RenderBuffer.Init(gl, 2, 0, 2, 40, 60)
        sprite_buffers["doodad"] = RenderBuffer.Init(gl, 2, 0, 2, 40, 60)
        sprite_buffers["item"] = RenderBuffer.Init(gl, 2, 0, 2, 80, 120)

        let screen = RenderBuffer.Init(gl, 2, 0, 2, 4, 6)

        let inv = 1.0 / 128.0

        SPRITES["you"] = new Map()
        let you_idle = new Sprite(0, 0, 16, 30, inv)
        let you_walk = new Sprite(18, 0, 12, 31, inv)
        let you_hurt = new Sprite(70, 0, 16, 29, inv)
        SPRITES["you"]["idle"] = [you_idle]
        SPRITES["you"]["walk"] = [you_walk, new Sprite(33, 0, 15, 30, inv), you_walk, you_idle]
        SPRITES["you"]["crouch"] = [new Sprite(51, 0, 16, 23, inv)]
        SPRITES["you"]["damaged"] = [you_hurt]
        SPRITES["you"]["death"] = [new Sprite(88, 0, 32, 15, inv)]
        SPRITES["you"]["attack"] = [new Sprite(0, 32, 32, 30, inv, -8, 0), new Sprite(33, 32, 32, 30, inv, -8, 0), new Sprite(66, 32, 44, 29, inv, 14, 0)]
        SPRITES["you"]["crouch-attack"] = [new Sprite(78, 64, 32, 31, inv, -8, -8), new Sprite(45, 64, 32, 23, inv, -8, 0), new Sprite(0, 64, 44, 22, inv, 14, 0)]

        SPRITES["skeleton"] = new Map()
        let skeleton_idle = new Sprite(0, 0, 16, 31, inv)
        SPRITES["skeleton"]["idle"] = [skeleton_idle]
        SPRITES["skeleton"]["walk"] = [new Sprite(17, 0, 16, 32, inv), skeleton_idle]
        SPRITES["skeleton"]["death"] = [new Sprite(35, 0, 16, 16, inv), new Sprite(53, 0, 15, 7, inv)]

        SPRITES["doodad"] = new Map()
        SPRITES["doodad"]["bone"] = [new Sprite(0, 0, 15, 13, inv)]
        SPRITES["doodad"]["splat"] = [new Sprite(23, 0, 8, 10, inv), new Sprite(32, 0, 3, 13, inv), new Sprite(36, 0, 6, 13, inv), new Sprite(43, 0, 8, 15, inv)]

        SPRITES["item"] = new Map()
        SPRITES["item"]["water"] = [new Sprite(0, 0, 16, 16, inv)]
        SPRITES["item"]["whip"] = [new Sprite(17, 0, 17, 17, inv)]
        SPRITES["item"]["musket"] = [new Sprite(35, 0, 15, 14, inv)]
        SPRITES["item"]["helmet"] = [new Sprite(53, 0, 13, 15, inv)]
        SPRITES["item"]["armor"] = [new Sprite(68, 0, 14, 15, inv)]
        SPRITES["item"]["boots"] = [new Sprite(86, 0, 12, 13, inv)]
        SPRITES["item"]["musket-ball"] = [new Sprite(100, 0, 8, 8, inv)]
        SPRITES["item"]["gloves"] = [new Sprite(111, 0, 14, 15, inv)]
        SPRITES["item"]["shield"] = [new Sprite(1, 19, 10, 15, inv)]
        SPRITES["item"]["food"] = [new Sprite(14, 19, 13, 12, inv)]
        SPRITES["item"]["roar"] = [new Sprite(29, 21, 14, 10, inv)]

        SPRITES["ui"] = new Map()
        SPRITES["ui"]["health"] = new Sprite(27, 33, 1, 6, inv)
        SPRITES["ui"]["stamina"] = new Sprite(29, 33, 1, 6, inv)
        SPRITES["ui"]["reduce"] = new Sprite(31, 33, 1, 6, inv)
        SPRITES["ui"]["gone"] = new Sprite(33, 33, 1, 6, inv)
        SPRITES["ui"]["panel"] = new Sprite(0, 0, 20, 32, inv)
        SPRITES["ui"]["select-panel"] = new Sprite(42, 0, 20, 32, inv)

        let world = new World()
        Network.Request("resources/map.json", (data) => {
            world.load(gl, data)
            for (let i = 0; i < this.world.thing_count; i++) {
                if (this.world.things[i].sprite_id === "you") {
                    self.player = this.world.things[i]
                    break
                }
            }
        })

        MUSIC["melody"] = new Audio("resources/vampire-killer.ogg")
        MUSIC["melody"].loop = true

        SOUND["pickup"] = new Audio("resources/pickup.wav")
        SOUND["destroy"] = new Audio("resources/destroy.wav")
        SOUND["you-hurt"] = new Audio("resources/you-hurt.wav")
        SOUND["you-whip"] = new Audio("resources/you-whip.wav")
        SOUND["levelup"] = new Audio("resources/levelup.wav")
        SOUND["roar"] = new Audio("resources/destroy.wav")

        window.onblur = function () {
            self.on = false
            self.music.pause()
        }
        window.onfocus = function () {
            self.on = true
            self.music.play()
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
        this.player = null
        this.sprite_buffers = sprite_buffers
        this.on = true
        this.gl = gl
        this.g = g
        this.generic = generic
        this.generic2 = generic2
        this.world = world
        this.music = MUSIC["melody"]

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
        if (this.player === null) {
            setTimeout(run, 500)
            return
        }
        document.body.appendChild(this.canvas)
        this.music.play()
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
        g.set_program(gl, "texture")
        g.set_orthographic(this.draw_ortho, view_x, view_y)
        g.update_mvp(gl)
        this.world.render(g, gl, frame, player.x, player.y, this.sprite_buffers)

        g.set_orthographic(this.draw_ortho, 0, 0)
        g.update_mvp(gl)
        g.set_texture(gl, "ui")
        this.generic.zero()

        Render.Sprite(this.generic, 20, 32, SPRITES["ui"]["panel"])
        Render.Sprite(this.generic, 62, 32, SPRITES["ui"]["panel"])
        Render.Sprite(this.generic, 41, 49, SPRITES["ui"]["panel"])
        Render.Sprite(this.generic, 41, 16, SPRITES["ui"]["panel"])

        let health_bar = player.health
        let health_gone = (player.health_lim - player.health)

        let stamina_bar = player.stamina
        let stamina_gone = (player.stamina_lim - player.stamina)

        let x = 20
        let y = 26
        Render.ImageSprite(this.generic, x + health_bar, frame.height - y, SPRITES["ui"]["gone"], health_gone, 6)
        if (player.health_reduce > player.health) {
            let health_reduce = player.health_reduce - health_bar
            Render.ImageSprite(this.generic, x + health_bar, frame.height - y, SPRITES["ui"]["reduce"], health_reduce, 6)
        }
        Render.ImageSprite(this.generic, x, frame.height - y, SPRITES["ui"]["health"], health_bar, 6)

        y = 33
        Render.ImageSprite(this.generic, x + stamina_bar, frame.height - y, SPRITES["ui"]["gone"], stamina_gone, 6)
        if (player.stamina_reduce > player.stamina) {
            let stamina_reduce = player.stamina_reduce - stamina_bar
            Render.ImageSprite(this.generic, x + stamina_bar, frame.height - y, SPRITES["ui"]["reduce"], stamina_reduce, 6)
        }
        Render.ImageSprite(this.generic, x, frame.height - y, SPRITES["ui"]["stamina"], stamina_bar, 6)

        RenderSystem.UpdateAndDraw(gl, this.generic)

        g.set_texture(gl, "item")
        this.generic.zero()
        if (player.hand !== null) {
            let sprite = player.hand.sprite[0]
            let x = 20 + 10 - sprite.width * 0.5
            let y = 32 + 16 - sprite.height * 0.5
            Render.Sprite(this.generic, x, y, sprite)
        }
        if (player.offhand !== null) {
            let sprite = player.offhand.sprite[0]
            let x = 62 + 10 - sprite.width * 0.5
            let y = 32 + 16 - sprite.height * 0.5
            Render.Sprite(this.generic, x, y, sprite)
        }
        if (player.item !== null) {
            let sprite = player.item.sprite[0]
            let x = 41 + 10 - sprite.width * 0.5
            let y = 16 + 16 - sprite.height * 0.5
            Render.Sprite(this.generic, x, y, sprite)
        }
        if (player.skill !== null) {
            let sprite = player.skill.sprite[0]
            let x = 41 + 10 - sprite.width * 0.5
            let y = 49 + 16 - sprite.height * 0.5
            Render.Sprite(this.generic, x, y, sprite)
        }
        RenderSystem.UpdateAndDraw(gl, this.generic)

        if (player.menu !== null)
            player.menu.render(g, gl, frame, this.generic, this.generic2)

        RenderSystem.SetFrameBuffer(gl, null)
        RenderSystem.SetView(gl, 0, 0, this.canvas.width, this.canvas.height)
        g.set_program(gl, "texture")
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