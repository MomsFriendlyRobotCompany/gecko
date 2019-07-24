
# capnp compile -o c++ vec.capnp
# this command generates:
#     vec.capnp.h/c++

# this was generated by the command above the first time i ran it
@0xd05f40fe642f2e3e;

using Common = import "msg.capnp";

struct IMU {
    accel      @0 :Common.Vector;
    gyro       @1 :Common.Vector;
    mag        @2 :Common.Vector;
    timestamp  @3 :Float64;
}
