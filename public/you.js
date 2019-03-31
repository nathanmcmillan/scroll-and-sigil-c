const InputOpNewMove = 0
const InputOpContinueMove = 1
const InputOpMissile = 2

class You extends Human {
    constructor(world, nid, x, y, z, angle, health, status) {
        super(world, nid, x, y, z, angle, health, status)
        this.camera = null
    }
    Walk() {
        let direction = null
        let goal = null

        // TODO filter input into map to reduce uploaded traffic

        if (Input.Is(" ")) {
            SocketSend.setUint8(SocketSendIndex, InputOpMissile, true)
            SocketSendIndex++
            SocketSendOperations++

            // this.Status = HumanMissile
            // this.AnimationMod = 0
            // this.AnimationFrame = 0
            // this.Animation = HumanAnimationMissile

            // PlaySound("baron-missile")
            return
        }

        if (Input.Is("w")) {
            direction = "w"
            goal = this.camera.ry
        }

        if (Input.Is("s")) {
            if (direction === null) {
                direction = "s"
                goal = this.camera.ry + Math.PI
            } else {
                direction = null
                goal = null
            }
        }

        if (Input.Is("a")) {
            if (direction === null) {
                direction = "a"
                goal = this.camera.ry - HalfPi
            } else if (direction === "w") {
                direction = "wa"
                goal -= QuarterPi
            } else if (direction === "s") {
                direction = "sa"
                goal += QuarterPi
            }
        }

        if (Input.Is("d")) {
            if (direction === null)
                goal = this.camera.ry + HalfPi
            else if (direction === "a")
                goal = null
            else if (direction === "wa")
                goal = this.camera.ry
            else if (direction === "sa")
                goal = this.camera.ry + Math.PI
            else if (direction === "w")
                goal += QuarterPi
            else if (direction === "s")
                goal -= QuarterPi
        }

        if (goal === null) {
            this.AnimationMod = 0
            this.AnimationFrame = 0
            this.Animation = HumanAnimationIdle
        } else {
            if (goal < 0)
                goal += Tau
            else if (goal >= Tau)
                goal -= Tau

            if (this.Angle !== goal) {
                this.Angle = goal
                SocketSend.setUint8(SocketSendIndex, InputOpNewMove, true)
                SocketSendIndex++
                SocketSend.setFloat32(SocketSendIndex, this.Angle, true)
                SocketSendIndex += 4
                SocketSendOperations++
            } else {
                SocketSend.setUint8(SocketSendIndex, InputOpContinueMove, true)
                SocketSendIndex++
                SocketSendOperations++
            }

            // TODO improve
            // this.X += Math.sin(this.Angle) * this.Speed * NetworkConversionRate
            // this.Z -= Math.cos(this.Angle) * this.Speed * NetworkConversionRate

            if (this.Animation === HumanAnimationIdle)
                this.Animation = HumanAnimationWalk

            if (this.UpdateAnimation() === AnimationDone)
                this.AnimationFrame = 0
        }
    }
    Update() {
        switch (this.Status) {
            case HumanDead:
                this.Dead()
            case HumanMissile:
                this.Missile()
                break
            default:
                this.Walk()
                break
        }
    }
}
