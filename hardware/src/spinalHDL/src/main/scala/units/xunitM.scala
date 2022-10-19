package units

import scala.collection.mutable.ArrayBuffer

import spinal.core._
import spinal.lib._

import scala.util.Random


//Hardware definition
class xunitM(data_w: Int) extends Component {
  val clk = in Bool()
  val rst = in Bool()
  val run = in Bool()
  val delay0 = in UInt(8 bits)
  val in0 = in UInt(data_w bits)
  val out0 = out UInt(data_w bits)

  // Configure the clock domain
  val xunitMClockDomain = ClockDomain(
    clock = clk,
    reset = rst,
    config = ClockDomainConfig(
      clockEdge = RISING,
      resetKind = ASYNC,
      resetActiveLevel = HIGH
    )
  )

  // All logic inside this area uses the same Clocking Area
  val coreArea = new ClockingArea(xunitMClockDomain) {

    val w = ArrayBuffer[UInt]()
    for(i <- 0 to 15) {
      w += Reg(UInt(data_w bits)) init(0)
    }

    val next_w = UInt(data_w bits)

    next_w := sigma1_32(w(14)) + w(9) + sigma0_32(w(1)) + w(0)

    val delay = Reg(UInt(8 bits)) init(0)
    val latency = Reg(UInt(5 bits)) init(0)
    val out0_reg = Reg(UInt(data_w bits))
    when(run) {
      delay := delay0
      latency := 17 // 5'h11
    } elsewhen (delay > 0) {
      delay := delay - 1
    } otherwise {
      when(latency > 0) {
        latency := latency - 1
      }

      for(i <- 0 to 14) {
        w(i) := w(i+1)
      }

      when(latency > 1){
        w(15) := in0
      } otherwise {
        w(15) := next_w
      }

    }
    // register output
    out0_reg := next_w
    out0 := out0_reg
  }

  def ROTR_32(x: UInt, c: Int): UInt = {
    return x.rotateRight(c)
  }

  def SHR(x: UInt, c: Int): UInt = {
    return (x |>> c)
  }

  def sigma0_32(x: UInt): UInt = {
    return ROTR_32(x, 7) ^ ROTR_32(x, 18) ^ SHR(x, 3)
  }

  def sigma1_32(x: UInt): UInt = {
    return ROTR_32(x, 17) ^ ROTR_32(x, 19) ^ SHR(x, 10)
  }
}

//Generate the xunitM's Verilog
object xunitM {
  def main(args: Array[String]) {
    SpinalConfig(targetDirectory = "rtl").generateVerilog(new xunitM(data_w = 32))
  }
}
