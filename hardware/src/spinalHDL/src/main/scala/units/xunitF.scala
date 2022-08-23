package units

import scala.collection.mutable.ArrayBuffer

import spinal.core._
import spinal.lib._

import scala.util.Random


//Hardware definition
class xunitF(data_w: Int) extends Component {
  val clk = in Bool()
  val rst = in Bool()
  val run = in Bool()
  val delay0 = in UInt(8 bits)
  val in0 = in UInt(data_w bits)
  val in1 = in UInt(data_w bits)
  val in2 = in UInt(data_w bits)
  val in3 = in UInt(data_w bits)
  val in4 = in UInt(data_w bits)
  val in5 = in UInt(data_w bits)
  val in6 = in UInt(data_w bits)
  val in7 = in UInt(data_w bits)
  val in8 = in UInt(data_w bits)
  val in9 = in UInt(data_w bits)
  val out0 = out UInt(data_w bits)
  val out1 = out UInt(data_w bits)
  val out2 = out UInt(data_w bits)
  val out3 = out UInt(data_w bits)
  val out4 = out UInt(data_w bits)
  val out5 = out UInt(data_w bits)
  val out6 = out UInt(data_w bits)
  val out7 = out UInt(data_w bits)

  // Configure the clock domain
  val xunitFClockDomain = ClockDomain(
    clock = clk,
    reset = rst,
    config = ClockDomainConfig(
      clockEdge = RISING,
      resetKind = ASYNC,
      resetActiveLevel = HIGH
    )
  )

  // All logic inside this area uses the same Clocking Area
  val coreArea = new ClockingArea(xunitFClockDomain) {
    val a, b, c, d, e, f, g, h = Reg(UInt(32 bits)) init(0)

    val w = UInt(32 bits)
    val k = UInt(32 bits)

    w := in8
    k := in9

    val T1 = UInt(data_w bits)
    val T2 = UInt(data_w bits)

    T1 := h + Sigma1_32(e) + Ch(e,f,g) + k + w
    T2 := Sigma0_32(a) + Maj(a,b,c)

    val T1_init = UInt(data_w bits)
    val T2_init = UInt(data_w bits)

    T1_init := in7 + Sigma1_32(in4) + Ch(in4,in5,in6) + k + w
    T2_init := Sigma0_32(in0) + Maj(in0,in1,in2)

    val delay = Reg(UInt(8 bits)) init(0)
    val latency = Reg(UInt(5 bits)) init(0)
    when(run) {
      delay := delay0
      latency := 1
    } elsewhen (delay > 0) {
      delay := delay - 1
    } otherwise {
      when(latency > 0) {
        latency := latency - 1
      }

      when(latency === 1){
        a := T1_init + T2_init
        b := in0
        c := in1
        d := in2
        e := in3 + T1_init
        f := in4
        g := in5
        h := in6
      }.otherwise {
        a := T1 + T2
        b := a
        c := b
        d := c
        e := d + T1
        f := e
        g := f
        h := g
      }    
    }

    // output
    out0 := a
    out1 := b
    out2 := c
    out3 := d
    out4 := e
    out5 := f
    out6 := g
    out7 := h
  }

  def ROTR_32(x: UInt, c: Int): UInt = {
    return x.rotateRight(c)
  }

  def Ch(x: UInt, y: UInt, z: UInt): UInt = {
    return ( (x & y) ^ ((~x) & z) )
  }

  def Maj(x: UInt, y: UInt, z: UInt): UInt = {
    return ( (x & y) ^ (x & z) ^ (y & z) )
  }

  def Sigma0_32(x: UInt): UInt = {
    return ROTR_32(x, 2) ^ ROTR_32(x, 13) ^ ROTR_32(x, 22)
  }

  def Sigma1_32(x: UInt): UInt = {
    return ROTR_32(x, 6) ^ ROTR_32(x, 11) ^ ROTR_32(x, 25)
  }
}

//Generate the xunitF's Verilog
object xunitF {
  def main(args: Array[String]) {
    SpinalConfig(targetDirectory = "rtl").generateVerilog(new xunitF(data_w = 32))
  }
}
