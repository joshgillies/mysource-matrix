<?  ##############################################
   ### SQUIZLIB ------------------------------###
  ##- Generic Include Files -- PHP4 ----------##
 #-- Copyright Squiz.net ---------------------#
##############################################
## This file is subject to version 1.0 of the
## MySource License, that is bundled with
## this package in the file LICENSE, and is
## available at through the world-wide-web at
## http://mysource.squiz.net/
## If you did not receive a copy of the MySource
## license and are unable to obtain it through
## the world-wide-web, please contact us at
## mysource@squiz.net so we can mail you a copy
## immediately.
##
## File: colour/examples/go.php
## Desc: Example usage of colour manipulation functions
## $Source: /home/csmith/conversion/cvs/mysource_matrix/core/mysource_matrix/fudge/colour/examples/Attic/go.php,v $
## $Revision: 1.1.1.1 $
## $Author: brobertson $
## $Date: 2003/03/21 00:47:12 $
#######################################################################

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