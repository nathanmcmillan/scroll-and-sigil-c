package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"math"
	"strconv"
	"strings"
)

// Animation constants
const (
	HumanWalkAnimation    int = 2
	HumanMeleeAnimation   int = 2
	HumanMissileAnimation int = 3
	HumanDeathAnimation   int = 2
)

// Human constants
const (
	HumanDead    = 0
	HumanWalk    = 1
	HumanMelee   = 2
	HumanMissile = 3
)

// You struct
type You struct {
	*Thing
	Status      int
	DeltaMoveXZ bool
	DeltaMoveY  bool
	DeltaAngle  bool
	Person      *Person
}

// NewYou func
func NewYou(world *World, person *Person, x, y, z float32) *You {
	you := &You{}
	you.Thing = &Thing{}
	you.UID = HumanUID
	you.NID = NextNID()
	you.World = world
	you.Thing.Update = you.Update
	you.Thing.Damage = you.Damage
	you.Thing.Snap = you.Snap
	you.Thing.Save = you.Save
	you.Thing.SnapBinary = you.SnapBinary
	you.Thing.SaveBinary = you.SaveBinary
	you.X = x
	you.Y = y
	you.Z = z
	you.Radius = 0.4
	you.Height = 1.0
	you.Speed = 0.1
	you.Health = 1
	you.Status = HumanWalk
	you.Person = person
	world.AddThing(you.Thing)
	you.BlockBorders()
	you.AddToBlocks()
	return you
}

// SaveBinary func
func (me *You) SaveBinary(raw *bytes.Buffer) {
	binary.Write(raw, binary.LittleEndian, me.UID)
	binary.Write(raw, binary.LittleEndian, me.NID)
	binary.Write(raw, binary.LittleEndian, float32(me.X))
	binary.Write(raw, binary.LittleEndian, float32(me.Y))
	binary.Write(raw, binary.LittleEndian, float32(me.Z))
	binary.Write(raw, binary.LittleEndian, float32(me.Angle))
	binary.Write(raw, binary.LittleEndian, uint16(me.Health))
}

// SnapBinary func
func (me *You) SnapBinary(raw *bytes.Buffer) int {
	binary.Write(raw, binary.LittleEndian, me.NID)
	binary.Write(raw, binary.LittleEndian, float32(me.X))
	binary.Write(raw, binary.LittleEndian, float32(me.Y))
	binary.Write(raw, binary.LittleEndian, float32(me.Z))
	binary.Write(raw, binary.LittleEndian, float32(me.Angle))
	binary.Write(raw, binary.LittleEndian, uint16(me.Health))
	return 1
}

// Save func
func (me *You) Save(snap *strings.Builder) {
	snap.WriteString("{u:")
	snap.WriteString(strconv.Itoa(int(me.UID)))
	snap.WriteString(",n:")
	snap.WriteString(strconv.Itoa(int(me.NID)))
	snap.WriteString(",x:")
	snap.WriteString(strconv.FormatFloat(float64(me.X), 'f', -1, 32))
	snap.WriteString(",y:")
	snap.WriteString(strconv.FormatFloat(float64(me.Y), 'f', -1, 32))
	snap.WriteString(",z:")
	snap.WriteString(strconv.FormatFloat(float64(me.Z), 'f', -1, 32))
	snap.WriteString(",a:")
	snap.WriteString(strconv.FormatFloat(float64(me.Angle), 'f', -1, 32))
	snap.WriteString(",h:")
	snap.WriteString(strconv.Itoa(me.Health))
	snap.WriteString("}")
}

// Snap func
func (me *You) Snap(snap *strings.Builder) {
	snap.WriteString("{n:")
	snap.WriteString(strconv.Itoa(int(me.NID)))
	if me.DeltaMoveXZ {
		snap.WriteString(",x:")
		snap.WriteString(strconv.FormatFloat(float64(me.X), 'f', -1, 32))
		snap.WriteString(",z:")
		snap.WriteString(strconv.FormatFloat(float64(me.Z), 'f', -1, 32))
		me.DeltaMoveXZ = false
	}
	// if me.DeltaMoveY {
	snap.WriteString(",y:")
	snap.WriteString(strconv.FormatFloat(float64(me.Y), 'f', -1, 32))
	// me.DeltaMoveY = false
	// }
	if me.DeltaAngle {
		snap.WriteString(",a:")
		snap.WriteString(strconv.FormatFloat(float64(me.Angle), 'f', -1, 32))
		me.DeltaAngle = false
	}
	if me.DeltaHealth {
		snap.WriteString(",h:")
		snap.WriteString(strconv.Itoa(me.Health))
		me.DeltaHealth = false
	}
	snap.WriteString("},")
}

// Damage func
func (me *You) Damage(amount int) {
	if me.Status != HumanDead {
		fmt.Println("ouch!", amount)
		me.DeltaHealth = true
	}
}

// Missile func
func (me *You) Missile() {
	anim := me.UpdateAnimation()
	if anim == AnimationAlmostDone {
		const speed = 0.3
		angle := float64(me.Angle)
		dx := float32(math.Sin(angle))
		dz := -float32(math.Cos(angle))
		x := me.X + dx*me.Radius*3.0
		y := me.Y + me.Height*0.75
		z := me.Z + dz*me.Radius*3.0
		NewPlasma(me.World, 1+NextRandP()%3, x, y, z, dx*speed, 0.0, dz*speed)
	} else if anim == AnimationDone {
		me.Status = HumanWalk
		me.AnimationFrame = 0
		me.Animation = HumanWalkAnimation
	}
}

// Walk func
func (me *You) Walk() {
	person := me.Person
	if person != nil && person.InputCount > 0 {
		move := false
		attack := false
		for i := 0; i < person.InputCount; i++ {
			input := person.InputQueue[i]
			if input == "b" {
				if !attack {
					me.Status = HumanMissile
					me.AnimationMod = 0
					me.AnimationFrame = 0
					me.Animation = HumanMissileAnimation
					attack = true
				}
			} else if !move {
				if input == "m" {
					me.DX += float32(math.Sin(float64(me.Angle))) * me.Speed
					me.DZ -= float32(math.Cos(float64(me.Angle))) * me.Speed
					move = true
					me.DeltaMoveXZ = true
				} else if strings.HasPrefix(input, "a:") {
					angle := strings.Split(input, "a:")[1]
					value, _ := strconv.ParseFloat(angle, 32)
					me.Angle = float32(value)
					me.DeltaAngle = true

					me.DX += float32(math.Sin(float64(me.Angle))) * me.Speed
					me.DZ -= float32(math.Cos(float64(me.Angle))) * me.Speed
					move = true
					me.DeltaMoveXZ = true
				}
			}
		}
		person.InputCount = 0
	}
}

// Update func
func (me *You) Update() bool {
	switch me.Status {
	case HumanMissile:
		me.Missile()
	default:
		me.Walk()
	}
	me.Integrate()
	return false
}
