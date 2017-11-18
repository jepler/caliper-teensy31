wm override . 1
wm geo . -64+0

font create BigFont {*}[font configure TkFixedFont] -size 48
font create SmallFont {*}[font configure TkFixedFont] -size 8
pack [label .l -font BigFont -width 7 -text "NO DATA" -justify right -anchor e -textv data] -side left
pack [label .u -font SmallFont -anchor w -textv unit] -side left -anchor nw

proc onread {serport} {
    global data
    global unit
    global lastchange
    set olddata $data
    set data  [string trim [chan read $serport] "\n"]
    set now [clock milliseconds]
    if {$data == $olddata} {
        if {$now > $lastchange + 1000} { .l configure -fg green }
        if {$now > $lastchange + 4000} { wm wi . }
    } else {
        .l configure -fg red
        set lastchange $now
        wm dei .
    }
    switch -glob -- $data {
        *.???? { set unit in }
        *.?? { set unit mm }
        * { set unit ?? }
    }
}

set data {}
set unit ??
set lastchange 0

set serport [open [lindex $argv 0]]
chan configure $serport -mode 9600,n,8,1 -buffering line -encoding binary -blocking false
chan event $serport readable [list onread $serport]

