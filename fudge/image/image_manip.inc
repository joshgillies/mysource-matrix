<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: image_manip.inc,v 1.9.8.1 2013/07/31 04:35:58 ewang Exp $
*
*/


require_once dirname(__FILE__).'/../general/file_system.inc';

/**
* Image_Manip
*
* Image manipulation functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.9.8.1 $
* @package Fudge
* @subpackage Image
*/
class Image_Manip
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
	* @return boolean
	*/
	public static function gradientPalette($input, $output, $colour_a, $colour_b)
	{
		require_once dirname(__FILE__).'/../colour/colour.inc';

		if (!($image = Image_Manip::fromFile($input))) {
			trigger_error('Unable to open image for recolourizing: '.$input);
			return false;
		}

		// Anylize colours and calculate difference colour
		$ca_array = Colour::htmlToRgb($colour_a);
		$cb_array = Colour::htmlToRgb($colour_b);
		$diff = Array(
					'r'	=> ($cb_array['r'] - $ca_array['r']),
					'g'	=> ($cb_array['g'] - $ca_array['g']),
					'b'	=> ($cb_array['b'] - $ca_array['b']),
				);

		// Get the number of colours in a palette
		$palette_size = imageColorsTotal($image);
		// Replace colours in palette
		for ($i = 0; $i < $palette_size; $i++) {
			// because the image is greyscale each value for the R, G & B will be the same
			// therefore to find the percentage white this index is we divide by 256
			$colour = imageColorsForIndex ($image, $i);
			$fraction = (double) ($colour['red'] / 256);

			$r = (int) (255 * ($ca_array['r'] + $diff['r'] * $fraction));
			$g = (int) (255 * ($ca_array['g'] + $diff['g'] * $fraction));
			$b = (int) (255 * ($ca_array['b'] + $diff['b'] * $fraction));
			imageColorSet($image,$i, $r, $g, $b);
		}

		// Output result to file
		$ret_val = Image_Manip::toFile($image, $output);
		imageDestroy($image);
		return $ret_val;

	}//end gradientPalette()


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
	* @return boolean
	*/
	public static function remapColour($input, $output, $colour_map, $tolerance=0.2)
	{
		require_once dirname(__FILE__).'/../colour/colour.inc';

		if (!($image = Image_Manip::fromFile($input))) {
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
		$ret_val = Image_Manip::toFile($image, $output);
		imageDestroy($image);
		return $ret_val;

	}//end remapColour()


	/**
	* Resizes an image
	*
	* @param string	$input		the input image file
	* @param string	$output		the output image file
	* @param int	$w			width of new image
	* @param int	$h			height of new image
	*
	* @return boolean
	*/
	public static function resize($input, $output, $w, $h, $input_x = 0, $input_y = 0, $input_w = 0, $input_h = 0)
	{
		// NOTE: $s_* = source image vars, $d_* = destination image vars
		$s_img = Image_Manip::fromFile($input);

		// transparency needs to be handled separately for GIF/PNG
		switch (get_file_type($output)) {
		case 'gif':
			$d_img = imageCreate($w, $h);
			// preserve GIF transparency, if any
			$colorTransparent = imagecolortransparent($s_img);
			if ($colorTransparent > -1) {
				imagepalettecopy($d_img,$s_img);
				imagefill($d_img,0,0,$colorTransparent);
				imagecolortransparent($d_img, $colorTransparent);
			}
			break;
		case 'png':
			// preserve PNG transparency
			$d_img = imageCreateTrueColor($w, $h);
			imagealphablending( $d_img, false );
			imagesavealpha( $d_img, true );
			break;
		default:
			$d_img = imageCreateTrueColor($w, $h);
		}

		$ret_val = false;

		if ($input_w == 0) {
			$input_w = imageSx($s_img);
		}
		if ($input_h == 0) {
			$input_h = imageSy($s_img);
		}

		$ret_val = ImageCopyResampled($d_img, $s_img, 0, 0, $input_x, $input_y, $w, $h, $input_w, $input_h);
		imageDestroy($s_img); // kill this to save memory before save

		if ($ret_val) {
			$ret_val = Image_Manip::toFile($d_img, $output);
		}

		// finally destory dest image
		imageDestroy($d_img);

		return $ret_val;

	}//end resize()


	/**
	* returns the image resource for the passed image file
	*
	* @param string	$input		the input image file
	*
	* @return mixed image resource or false
	*/
	public static function fromFile($input)
	{
		switch (get_file_type($input)) {
			case 'gif' :
				if ($image = imageCreateFromGif ($input)) {
					return $image;
				} else {
					trigger_error('Unable to open GIF : '.$input);
				}
			break;
			case 'png' :
				if ($image = imageCreateFromPNG($input)) {
					return $image;
				} else {
					trigger_error('Unable to open PNG : '.$input);
				}
			break;
			case 'jpg' :
			case 'jpeg':
				if ($image = imageCreateFromJPEG($input)) {
					return $image;
				} else {
					trigger_error('Unable to open JPEG : '.$input);
				}
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
	* @param resource	$image			the image resource to output
	* @param string		$output			the output image file
	* @param int		$jpeg_quality	number between 0 and 100
	*
	* @return boolean
	*/
	public static function toFile($image, $output, $jpeg_quality=80)
	{
		// Output result to file
		switch (get_file_type($output)) {
			case 'gif' :
				if (imageGif($image, $output)) return true;
			break;

			case 'png' :
				if (imagePNG($image, $output)) return true;
			break;

			case 'jpg' :
			case 'jpeg':
				if (imageJPEG($image, $output, $jpeg_quality)) {
					return TRUE;
				}
			break;

			default:
				trigger_error('Unable to write unknown image type : '.$output, E_USER_WARNING);
			break;
		}

		trigger_error('Unable to output image : '.$output, E_USER_WARNING);
		return FALSE;

	}//end toFile()


}//end class
?>
