package units

import spinal.core._
import spinal.lib._

import scala.util.Random

//Hardware definition
class SpinalxunitM extends Component {
  val io = new Bundle {
    val cond0 = in  Bool()
    val cond1 = in  Bool()
    val flag  = out Bool()
    val state = out UInt(8 bits)
  }
  val counter = Reg(UInt(8 bits)) init(0)

  when(io.cond0){
    counter := counter + 1
  }

  io.state := counter
  io.flag  := (counter === 0) | io.cond1
}

//Generate the SpinalxunitM's Verilog
object SpinalxunitM {
  def main(args: Array[String]) {
    SpinalConfig(targetDirectory = "rtl").generateVerilog(new SpinalxunitM)
  }
}
