<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_general.inc,v 1.5 2004/09/03 07:40:42 arailean Exp $
* $Name: not supported by cvs2svn $
*/


/**
* bodycopy_general.inc
*
* Contains a library of functions used by bodycopy assets
*
* @author mnyeholt
* @version $version$ - 1.0
* @package MySource_Matrix
* @subpackage Core
*/


/**
* Print a bodycopy style icon
*
* @param string $href		The HREF that this icons points to
* @param string $heading	The heading for the tooltip
* @param string $desc		The main body of the tooltip
* @param string $icon		The filename of the icon to display (must be a GIF file)
* @param string $extra		Any extras you want to put at the end of the IMG tag
* @param string $width		The width of the icon
* @param string $height		The height of the icon
*
* @access  public
* @return void
*/
function print_bodycopy_icon($href, $heading, $desc, $icon, $alt='', $extra='', $width='16', $height='16')
{
	$path = sq_web_path('data');

	$href = str_replace('"', '\"', $href);

	$tag = "
	<a href=\"$href\">
	<script language=\"JavaScript\" type=\"text/javascript\">
		sq_print_icon('$path/asset_types/bodycopy/images/icons/$icon.png', '$width', '$height', '');
	</script>
	</a>";

	$tag_with_tooltip = make_tooltip($tag, $desc, $heading);

	echo $tag_with_tooltip;

}//end print_bodycopy_icon()


?>
