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
* $Id: image.inc,v 1.10 2003/11/26 00:51:18 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../general/file_system.inc';

/**
* Image
*
* Image manipulation functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage Image
*/
class Image
{

	/**
	* Takes a link to an image file with an (assumed) greyscale palette 
	* Then replaces the Black<->White with ColourA<->ColourB         
	* And outputs an a new file                                      
	*
	* @param string	$input		the input image file
	* @param string	$output		the output image file
	* @param string	$colour_a	hex string or html colour name
	* @param string	$colour_b	hex string or html colour name
	*
	* @access public
	*
	*/
	function gradientPalette($input, $output, $colour_a, $colour_b) 
	{
		require_once dirname(__FILE__).'/../colour/colour.inc';

		if (!($image = Image::fromFile($input))) {
			trigger_error('Unable to open image for recolourizing: '.$input);
			return false;
		}

		// Anylize colours and calculate difference colour
		$ca_array = Colour::htmlToRgb($colour_a);
		$cb_array = Colour::htmlToRgb($colour_b);
		$diff = Array(
			'r' => ($cb_array['r'] - $ca_array['r']),
			'g' => ($cb_array['g'] - $ca_array['g']),
			'b' => ($cb_array['b'] - $ca_array['b'])
		);

		// Get the number of colours in a palette
		$palette_size = imageColorsTotal($image);
		// Replace colours in palette
		for ($i = 0; $i < $palette_size; $i++) {
			// because the image is greyscale each value for the R, G & B will be the same
			// therefore to find the percentage white this index is we divide by 256
			$colour = imagecolorsforindex ($image, $i);
			$fraction = (double) ($colour['red'] / 256);

			$r = (int) (255 * ($ca_array['r'] + $diff['r'] * $fraction));
			$g = (int) (255 * ($ca_array['g'] + $diff['g'] * $fraction));
			$b = (int) (255 * ($ca_array['b'] + $diff['b'] * $fraction));
			imagecolorset($image,$i, $r, $g, $b);
		}

		// Output result to file
		$ret_val = Image::toFile($image, $output);
		imageDestroy($image);
		return $ret_val;

	}// end gradientPalette()

	/**
	* Takes a link to an image file with an (assumed) greyscale palette 
	* $colour_map is an associative array whose 'key' colour in the image and replaces if with the 'value' colour.
	* The colours are in the form of HTML colour codes e.g. '5627e8', 'goldenrod'
	*
	* @param string	$input		the input image file
	* @param string	$output		the output image file
	* @param Array	$colour_map	It find the 'key' colour in the image and replaced if with the 'value' colour. Colours are in hex string or html colour name
	* @param float	$tolerance	
	*
	* @access public
	*
	*/
	function remapColour($input, $output, $colour_map, $tolerance=0.2) 
	{
		require_once dirname(__FILE__).'/../colour/colour.inc';

		if (!($image = Image::fromFile($input))) {
			trigger_error('Unable to open image for recolourizing: '.$input);
			return false;
		}

		// Get the number of colours in a palette
		$palette_size = imageColorsTotal($image);

		// Replace colours in palette
		for ($i = 0; $i < $palette_size; $i++) {
			$colour = imageColorsForIndex($image, $i);
			// Convert to HTML colour
			$colour = Colour::rgbToHtml(Colour::rgb($colour['red'],$colour['green'],$colour['blue']));
			$rgb = Colour::htmlToRgb(Colour::remap($colour,$colour_map,$tolerance));
			$r = $rgb['r'] * 255;
			$g = $rgb['g'] * 255;
			$b = $rgb['b'] * 255;
			imagecolorset($image,$i, $r, $g, $b);
		}

		// Output result to file
		$ret_val = Image::toFile($image, $output);
		imageDestroy($image);
		return $ret_val;

	}// remapColour()

	/**
	* Resizes an image 
	*
	* @param string	$input		the input image file
	* @param string	$output		the output image file
	* @param int	$w			width of new image
	* @param int	$h			height of new image
	*
	* @return bool
	* @access public
	*
	*/
	function resize($input, $output, $w, $h) 
	{

		$scaler  = 'pnmscale';
		$decoder = '';
		$encoder = '';

		// Check we have the right decoder
		switch(get_file_type($input)) {
			case 'gif':
				$decoder = 'giftopnm';
				$encoder = 'pnmtogif';
				break;
			case 'png':
				$decoder = 'pngtopnm';
				$encoder = 'pnmtopng';
				break;
			case 'jpg': 
			case 'jpeg':
				$decoder = 'djpeg';
				$encoder = 'cjpeg';
				break;
			default:
				trigger_error('Unable to open unknown image type for resizing: '.$input, E_USER_WARNING);
				return false;
		}

		$needed_programs[] = $encoder;
		$needed_programs[] = $decoder;

		foreach($needed_programs as $p) {
			$code = 0;
			system("$p /? 2> /dev/null",$code);
			if($code != 1) {
				trigger_error('Unable to resize image, required external program could not be found: '.$p, E_USER_WARNING);
				$die = true;
			}
		}
		if($die) return false;

		$options = '';

		if($w) $options .= ' -w $w';
		if($h) $options .= ' -h $h';

		$err = `(cat $input | $decoder | $scaler $options | $encoder > $output) 2>&1; echo $?`;
		if($err > 0) {
			trigger_error("There seem to have been problems resizing the image\n\n$input\n\nto\n\n$output:\n\n".htmlspecialchars($err));
			unlink($output);
			return false;
		}
		return true;
	}// end resize()

//	 ########################################################
//	# Resizes an image using bicubic interpolation inside PHP
//	# this doesn't appear to work without GD 2.0 or later
//	function new_image_resize($input, $output, $new_w, $new_h) {
//		
//		list($old_w, $old_h) = getimagesize($input);
//
//		if($new_w <= 0) $new_w = $old_w;
//		if($new_h <= 0) $new_h = $old_h;
//
//		$old_image = image_from_file($input);
//		$new_image = ImageCreateTrueColor($new_w, $new_h);
//
//		imagecopyresampled($new_image,$old_image, 0, 0, 0, 0, $new_w, $new_h, $old_w, $old_h);
//
//		image_to_file($new_image, $output);
//
//	}

	/**
	* returns the image resource for the passed image file
	*
	* @param string	$input		the input image file
	*
	* @return mixed int or false
	* @access public
	*/
	function fromFile($input) 
	{
		switch(get_file_type($input)) {
			case 'gif' :
				if ($image = ImageCreateFromGif($input)) return $image;
				else trigger_error('Unable to open GIF : '.$input);
				break;
			case 'png' :
				if ($image = ImageCreateFromPNG($input)) return $image;
				else trigger_error('Unable to open PNG : '.$input);
				break;
			case 'jpg' : 
			case 'jpeg':
				if ($image = ImageCreateFromJPEG($input)) return $image;
				else trigger_error('Unable to open JPEG : '.$input);
				break;
			default:
				trigger_error('Unable to open unknown image type : '.$input);
				break;
		}
		return false;

	}//end fromFile()

	/**
	* Write the image into the output file
	*
	* @param string	$input			the input image file
	* @param string	$output			the output image file
	* @param int	$jpeg_quality	number between 0 and 100
	*
	* @return mixed int or false
	* @access public
	*/
	function toFile($image, $output, $jpeg_quality=80) 
	{
		// Output result to file
		switch(get_file_type($output)) {
			case 'gif' :
				if (imageGIF($image, $output)) return true;
				break;
			case 'png' :
				if (imagePNG($image, $output)) return true;
				break;
			case 'jpg' :
			case 'jpeg':
				if (imageJPEG($image, $output, $jpeg_quality)) return true;
				break;
			default:
				trigger_error('Unable to write unknown image type : '.$input, E_USER_WARNING);
				break;
		}

		trigger_error('Unable to output image : '.$output, E_USER_WARNING);
		return false;

	}// end toFile()

}// end class
?>
