<? 

include_once("../colour.inc");

# Essentially these functions deal with HTML colour codes (minus
# the #). They allow you to do a number of interesting things with
# them. This is useful for generating matching colour schemes
# when supplied with one or two base colours by a user.
#
# See colour.inc itself for more information.

# Colours can be stored in a variety of formats:
#  * html_colour - e.g. "f22b4a","hotpink"
#    Functions relating to "html_colour" can handle the standard
#    texural labels for some colours (see code itself for a full
#    list).
#  * rgb  - e.g. array("r" => 0.3, "g" => 1.0, "b" => 0.7);
#    Red, Green, Blue values between 0 and 1.
#  * cmyk - e.g. array("c" => 0.4, "m" => 0, "y" => 0.1, "k" => 0.3);
#    Cyan, Magenta, Yellow, Black values between 0 and 1.
#  * hsv  - e.g. array("h" => 245, "s" => 0.7, "v" => 0.8);
#    Hue[0,360], Saturation[0,1], Value[0,1] values.
#  * int  - e.g. 5478934
#    3 bytes, most significant being red, then green, then blue.

$colour = "cornflowerblue";



?>