<?php

/**
* Colour
*
* Purpose
*     Colour manipulation functions
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage Colour
*/
class Colour 
{

	/**
	* Returns a colour palette with colour names to hex colours or a specific hex colour for the passed name 
	*
	* @param string	$name	a specific name to find in the palette
	*
	* @return mixed Array() or string if a specific name is passed
	* @access public
	*/
	function colourPalette($name=null) 
	{
		$palette = Array(
					'aliceblue'				=> 'f0f8ff',
					'antiquewhite'			=> 'faebd7',
					'aqua'					=> '00ffff',
					'aquamarine'			=> '7fffd4',
					'azure'					=> 'f0ffff',
					'beige'					=> 'f5f5dc',
					'bisque'				=> 'ffe4c4',
					'black'					=> '000000',
					'blanchedalmond'		=> 'ffebcd',
					'blue'					=> '0000ff',
					'blueviolet'			=> '8a2be2',
					'brown'					=> 'a52a2a',
					'burlywood'				=> 'deb887',
					'cadetblue'				=> '5f9ea0',
					'chartreuse'			=> '7fff00',
					'chocolate'				=> 'd2691e',
					'coral'					=> 'ff7f50',
					'cornflowerblue'		=> '6495ed',
					'cornsilk'				=> 'fff8dc',
					'crimson'				=> 'dc143c',
					'cyan'					=> '00ffff',
					'darkblue'				=> '00008b',
					'darkcyan'				=> '008b8b',
					'darkgoldenrod'			=> 'b8860b',
					'darkgray'				=> 'a9a9a9',
					'darkgreen'				=> '006400',
					'darkkhaki'				=> 'bdb76b',
					'darkmagenta'			=> '8b008b',
					'darkolivegreen'		=> '556b2f',
					'darkorange'			=> 'ff8c00',
					'darkorchid'			=> '9932cc',
					'darkred'				=> '8b0000',
					'darksalmon'			=> 'e9967a',
					'darkseagreen'			=> '8fbc8f',
					'darkslateblue'			=> '483d8b',
					'darkslategray'			=> '2f4f4f',
					'darkturquoise'			=> '00ced1',
					'darkviolet'			=> '9400d3',
					'deeppink'				=> 'ff1493',
					'deepskyblue'			=> '00bfff',
					'dimgray'				=> '696969',
					'dodgerblue'			=> '1e90ff',
					'firebrick'				=> 'b22222',
					'floralwhite'			=> 'fffaf0',
					'forestgreen'			=> '228b22',
					'fuchsia'				=> 'ff00ff',
					'gainsboro'				=> 'dcdcdc',
					'ghostwhite'			=> 'f8f8ff',
					'gold'					=> 'ffd700',
					'goldenrod'				=> 'daa520',
					'gray'					=> '808080',
					'green'					=> '008000',
					'greenyellow'			=> 'adff2f',
					'honeydew'				=> 'f0fff0',
					'hotpink'				=> 'ff69b4',
					'indianred'				=> 'cd5c5c',
					'indigo'				=> '4b0082',
					'ivory'					=> 'fffff0',
					'khaki'					=> 'f0e68c',
					'lavender'				=> 'e6e6fa',
					'lavenderblush'			=> 'fff0f5',
					'lawngreen'				=> '7cfc00',
					'lemonchiffon'			=> 'fffacd',
					'lightblue'				=> 'add8e6',
					'lightcoral'			=> 'f08080',
					'lightcyan'				=> 'e0ffff',
					'lightgoldenrodyellow'	=> 'fafad2',
					'lightgreen'			=> '90ee90',
					'lightgrey'				=> 'd3d3d3',
					'lightpink'				=> 'ffb6c1',
					'lightsalmon'			=> 'ffa07a',
					'lightseagreen'			=> '20b2aa',
					'lightskyblue'			=> '87cefa',
					'lightslategray'		=> '778899',
					'lightsteelblue'		=> 'b0c4de',
					'lightyellow'			=> 'ffffe0',
					'lime'					=> '00ff00',
					'limegreen'				=> '32cd32',
					'linen'					=> 'faf0e6',
					'magenta'				=> 'ff00ff',
					'maroon'				=> '800000',
					'mediumaquamarine'		=> '66cdaa',
					'mediumblue'			=> '0000cd',
					'mediumorchid'			=> 'ba55d3',
					'mediumpurple'			=> '9370db',
					'mediumseagreen'		=> '3cb371',
					'mediumslateblue'		=> '7b68ee',
					'mediumspringgreen'		=> '00fa9a',
					'mediumturquoise'		=> '48d1cc',
					'mediumvioletred'		=> 'c71585',
					'midnightblue'			=> '191970',
					'mintcream'				=> 'f5fffa',
					'mistyrose'				=> 'ffe4e1',
					'moccasin'				=> 'ffe4b5',
					'navajowhite'			=> 'ffdead',
					'navy'					=> '000080',
					'oldlace'				=> 'fdf5e6',
					'olive'					=> '808000',
					'olivedrab'				=> '6b8e23',
					'orange'				=> 'ffa500',
					'orangered'				=> 'ff4500',
					'orchid'				=> 'da70d6',
					'palegoldenrod'			=> 'eee8aa',
					'palegreen'				=> '98fb98',
					'paleturquoise'			=> 'afeeee',
					'palevioletred'			=> 'db7093',
					'papayawhip'			=> 'ffefd5',
					'peachpuff'				=> 'ffdab9',
					'peru'					=> 'cd853f',
					'pink'					=> 'ffc0cb',
					'plum'					=> 'dda0dd',
					'powderblue'			=> 'b0e0e6',
					'purple'				=> '800080',
					'red'					=> 'ff0000',
					'rosybrown'				=> 'bc8f8f',
					'royalblue'				=> '4169e1',
					'saddlebrown'			=> '8b4513',
					'salmon'				=> 'fa8072',
					'sandybrown'			=> 'f4a460',
					'seagreen'				=> '2e8b57',
					'seashell'				=> 'fff5ee',
					'sienna'				=> 'a0522d',
					'silver'				=> 'c0c0c0',
					'skyblue'				=> '87ceeb',
					'slateblue'				=> '6a5acd',
					'slategray'				=> '708090',
					'snow'					=> 'fffafa',
					'springgreen'			=> '00ff7f',
					'steelblue'				=> '4682b4',
					'tan'					=> 'd2b48c',
					'teal'					=> '008080',
					'thistle'				=> 'd8bfd8',
					'tomato'				=> 'ff6347',
					'turquoise'				=> '40e0d0',
					'violet'				=> 'ee82ee',
					'wheat'					=> 'f5deb3',
					'white'					=> 'ffffff',
					'whitesmoke'			=> 'f5f5f5',
					'yellow'				=> 'ffff00',
					'yellowgreen'			=> '9acd32'
				);

		if (is_null($name)) { 
			return $palette;
		} else { 
			return (empty($palette[$name])) ? '' : $palette[$name];
		}
	}// end colourPalette()


	/**
	* Create an RGB colour - either in 0 to 1 range or in 0 to 255 range
	*
	* @param int	$r					Red
	* @param int	$g					Green
	* @param int	$b					Blue
	* @param bool	$zero_to_one_range	when true value are 0 to 1 range
	*
	* @return Array()
	* @access public
	*
	*/
	function rgb($r, $g, $b, $zero_to_one_range=true)
	{

		if ($zero_to_one_range) {
			return Array(
				'r' => (($r >= 0) ? (($r <= 255) ? (double) ( ((double)$r) / 255.0) : 1) : 0),
				'g' => (($g >= 0) ? (($g <= 255) ? (double) ( ((double)$g) / 255.0) : 1) : 0),
				'b' => (($b >= 0) ? (($b <= 255) ? (double) ( ((double)$b) / 255.0) : 1) : 0)
			);
		} else {
			return Array(
				'r' => (($r >= 0) ? (($r <= 255) ? ((int) $r) : 255) : 0),
				'g' => (($g >= 0) ? (($g <= 255) ? ((int) $g) : 255) : 0),
				'b' => (($b >= 0) ? (($b <= 255) ? ((int) $b) : 255) : 0)
			);
		}
	}

	/**
	* Create a CMYK colour
	*
	* @param int	$c	Cyan
	* @param int	$m	Magenta
	* @param int	$y	Yellow
	* @param int	$k	blacK
	*
	* @return Array()
	* @access public
	*
	*/
	function cmyk($c, $m, $y, $k)
	{
		$output = Array(
			'c' => (($c >= 0) ? (($c <= 100) ? (double) ( ((double)$c) / 100.0) : 1) : 0),
			'm' => (($g >= 0) ? (($m <= 100) ? (double) ( ((double)$m) / 100.0) : 1) : 0),
			'y' => (($y >= 0) ? (($y <= 100) ? (double) ( ((double)$y) / 100.0) : 1) : 0),
			'k' => (($k >= 0) ? (($k <= 100) ? (double) ( ((double)$k) / 100.0) : 1) : 0)
		);
		Colour::cmykCorrect($output);
		return $output;
	}

	/**
	* Create a HSV colour
	*
	* @param float	$h	Hue
	* @param float	$s	Saturation
	* @param float	$v	Value
	*
	* @return Array()
	* @access public
	*
	*/
	function hsv($h,$s,$v)
	{
		return Array('h' => $h, 's' => $s/100, 'v' => $v/100);
	}

	/**
	* Converts CMYK to RGB - simple dodgy version
	*
	* @param array	$input	CMYK array
	*
	* @return Array()
	* @access public
	*
	*/
	function cmykToRgb($input)
	{
		$c = $input['c'];
		$m = $input['m'];
		$y = $input['y'];
		$k = $input['k'];
		$output = Array('r' => 0, 'g' => 0, 'b' => 0);

		if (($c + $k) < 1)
			$output['r'] = 1 - ($c + $k);
		else
			$output['r'] = 0;

		if (($m + $k) < 1)
			$output['g'] = 1 - ($m + $k);
		else
			$output['g'] = 0;

		if (($y + $k) < 1)
			$output['b'] = 1 - ($y + $k);
		else
			$output['b'] = 0;

		return $output;
	}

	/**
	* Converts RGB to CMYK - simple dodgy version
	*
	* @param array	$input	RGB array
	*
	* @return Array()
	* @access public
	*
	*/
	function rgbToCmyk($input)
	{
		$r = $input['r'];
		$g = $input['g'];
		$b = $input['b'];

		$c = 1 - $r;
		$m = 1 - $g;
		$y = 1 - $b;

		if ($c < $m)
			$k = $c;
		else
			$k = $m;

		if ($y < $k)
			$k = $y;

		if ($k > 0) {
			$c -= $k;
			$m -= $k;
			$y -= $k;
		}

		$output = Array('c' => $c, 'm' => $m, 'y' => $y, 'k' => $k);

		Colour::cmykCorrect($output);

		return $output;
	}

	/**
	* CMYK Colour corrector
	*
	* @param array	$input	CMYK array
	*
	* @return Array()
	* @access public
	*
	*/
	function cmykCorrect(&$input)
	{
		if ($input['c'] < $input['m'])
			$min = $input['c'];
		else
			$min = $input['m'];

		if ($input['y'] < $min)
			$min = $input['y'];

		if (($min + $input['k']) > 1)
			$min = 1 - $input['k'];

		$input['c'] -= $min;
		$input['m'] -= $min;
		$input['y'] -= $min;
		$input['k'] += $min;
	}

	/**
	* Inverts an RGB value
	*
	* @param array	$input	RGB array
	*
	* @return Array()
	* @access public
	*
	*/
	function rgbInvert($i)
	{
		return Array('r' => (1 - $i['r']), 'g' => (1 - $i['g']), 'b' => (1 - $i['b']));
	}

	/**
	* Converts an RGB colour to a hex string for html colours
	*
	* @param array	$input	RGB array
	*
	* @return string
	* @access public
	*
	*/
	function rgbToHtml($input)
	{
		return sprintf('%02x%02x%02x', $input['r'] * 255, $input['g'] * 255, $input['b'] * 255);
	}

	/**
	* Converts an integer to a html code
	*
	* @param int	$input	Some integer
	*
	* @return string
	* @access public
	*
	*/
	function intToHtml($input)
	{
		return sprintf('%06x', $input);
	}

	/**
	* Converts an integer to an rgb colour
	*
	* @param int	$input				Some integer
	* @param bool	$zero_to_one_range	when true value are 0 to 1 range
	*
	* @return Array()
	* @access public
	*
	*/
	function intToRgb($input, $zero_to_one_range=true)
	{
		$b = min($input % 256, 255);
		$g = min(($input >> 8) % 256, 255);
		$r = min($input >> 16, 255);
		return Colour::rgb($r, $g, $b, $zero_to_one_range);
	}

	/**
	* Converts a hex string/colour to a hex string
	*
	* @param string	$input	hex string or html colour name
	*
	* @return string
	* @access public
	*
	*/
	function htmlToHex($input)
	{
		// remove any hashes from the start
		$input = preg_replace('/^#/', '', strtolower($input));
		if (sprintf('%06x',hexdec($input)) != $input) {
			// Must be a colour name, or a fuck-up
			if ($hex = Colour::colourPalette($input) ) {
				$input = $hex;
			}
		}
		return $input;
	}

	/**
	* Converts a hex string/colour to a RGB colour
	*
	* @param string	$input				hex string or html colour name
	* @param bool	$zero_to_one_range	when true value are 0 to 1 range
	*
	* @return Array()
	* @access public
	*
	*/
	function htmlToRgb($input, $zero_to_one_range=true)
	{
		$input = Colour::htmlToHex($input);
		return Colour::rgb(
			hexdec(substr($input, 0, 2)),
			hexdec(substr($input, 2, 2)),
			hexdec(substr($input, 4, 2)),
			$zero_to_one_range
		);
	}

	/**
	* Converts a hex string/colour to an integer
	*
	* @param string	$input				hex string or html colour name
	*
	* @return Array()
	* @access public
	*
	*/
	function htmlToInt($input)
	{
		return hexdec(Colour::htmlToHex($input));
	}

	/**
	* Converts a rbg colour to an integer
	*
	* @param Array	$input				RBG colour
	*
	* @return Array()
	* @access public
	*
	*/
	function rgbToInt($input)
	{
		return Colour::htmlToInt(Colour::rgbToHtml($input));
	}

	/**
	* Converts an RGB colour to a HSV colour (Hue/Saturation/Value)
	* r,g,b values are from 0 to 1
	* h = [0,360], s = [0,1], v = [0,1]
	*		if s == 0, then h = -1 (undefined)
	*       h is cyclic
	*
	* @param Array	$input				RBG colour, with values from 0 to 1
	*
	* @return Array()
	* @access public
	*
	*/
	function rgbToHsv($input)
	{
		$r = max(0,min(1,$input['r']));
		$g = max(0,min(1,$input['g']));
		$b = max(0,min(1,$input['b']));
		$min = min($r,$g,$b);
		$max = max($r,$g,$b);
		if(!$max) { // Black
			return Array('h' => -1, 's' => 0, 'v' => 0);
		}
		$delta = $max - $min;
		$v = $max;
		$s = $delta / $max;
		if (!$s) { // Grey
			return Array('h' => -1, 's' => 0, 'v' => $v);
		}
		if ($r == $max) {
			// between yellow & magenta
			$h = ($g - $b) / $delta;
		} elseif ($g == $max) {
			// between cyan & yellow
			$h = 2 + ($b - $r) / $delta;
		} else {
			// between magenta & cyan
			$h = 4 + ($r - $g) / $delta;
		}
		$h *= 60; // degrees
		if($h < 0) $h += 360;
		return Array('h' => $h, 's' => $s, 'v' => $v);
	}

	/**
	* Converts an HSV colour to a RGB colour
	*
	* @param Array	$input				HSV colour
	*
	* @return Array()
	* @access public
	*
	*/
	function hsvToRgb($input)
	{
		$h = $input['h'] % 360;
		$s = max(0,min(1,$input['s']));
		$v = max(0,min(1,$input['v']));
		if (!$s) { // Grey
			return Array('r' => $v, 'g' => $v, 'b' => $v);
		}
		// Split into the six sections of the colour wheel
		$h /= 60;
		// Section number (0 - 5)
		$i  = floor($h);
		// Amount through the sector towards the next one [0,1]
		$f  = $h - $i;
		// Remember, $v is the value of the maximum colour [r,g,b]
		// $p is the minimum colour, according to the saturation
		$p  = $v * (1 - $s);
		// $q is the colour in between, assuming we're heading down
		$q  = $v * (1 - $s * $f );
		// $tq is the colour in between, assuming we're heading up
		$t  = $v * (1 - $s * (1 - $f ));
		switch($i) {
			case 0: // Between red and yellow
				$r = $v;
				$g = $t;
				$b = $p;
				break;
			case 1: // Between yellow and green
				$r = $q;
				$g = $v;
				$b = $p;
				break;
			case 2: // Between green and cyan
				$r = $p;
				$g = $v;
				$b = $t;
				break;
			case 3: // Between cyan and blue
				$r = $p;
				$g = $q;
				$b = $v;
				break;
			case 4: // Between blue and magenta
				$r = $t;
				$g = $p;
				$b = $v;
				break;
			case 5: default: // Between magenta and red
				$r = $v;
				$g = $p;
				$b = $q;
				break;
		}
		return Array('r' => $r, 'g' => $g, 'b' => $b);
	}

	/**
	* Converts a html colour to a HSV colour
	*
	* @param string	$input				hex string or html colour name
	*
	* @return Array()
	* @access public
	*
	*/
	function htmlToHsv($input)
	{
		return Colour::rgbToHsv(Colour::htmlToRgb($input));
	}

	/**
	* Converts a HSV colour to a html_colour
	*
	* @param Array()	$input				HSV colour
	*
	* @return string
	* @access public
	*
	*/
	function hsvToHtml($input)
	{
		return Colour::rgbToHtml(Colour::hsvToRgb($input));
	}

	/**
	* Takes a colour and returns black or white, depending on which contrasts the most
	*
	* @param string	$input				hex string or html colour name
	*
	* @return string
	* @access public
	*
	*/
	function contrastingShade($input)
	{
		$c = Colour::htmlToRgb($input);
		$weight_r    = $c['r'] * 0.75;
		$weight_g    = $c['g'] * 1.75; // Green's bright
		$weight_b    = $c['b'] * 0.5;  // Blue's dim
		if (($weight_r + $weight_g + $weight_b) >= 1.5) {
			return '000000';
		} else {
			return 'ffffff';
		}
	}

	/**
	* Takes a colour and returns a colour twice at light (half-way to white)
	*
	* @param string	$input				hex string or html colour name
	*
	* @return string
	* @access public
	*
	*/
	function twiceAsLight($input)
	{
		$c = Colour::htmlToRgb($input);
		$c['r'] += ((1 - $c['r']) / 2);
		$c['g'] += ((1 - $c['g']) / 2);
		$c['b'] += ((1 - $c['b']) / 2);
		return Colour::rgbToHtml($c);
	}

	/**
	* Takes a colour and returns a colour twice at dark (half-way to black)
	*
	* @param string	$input				hex string or html colour name
	*
	* @return string
	* @access public
	*
	*/
	function twiceAsDark($input)
	{
		$c = Colour::htmlToRgb($input);
		$c['r'] /= 2;
		$c['g'] /= 2;
		$c['b'] /= 2;
		return Colour::rgbToHtml($c);
	}


	/**
	* Adjust brightness on a colour, takes values between -1 and 1
	*
	* @param string	$input				hex string or html colour name
	* @param float	$amount				the amount of brightness (-1 to 1)
	*
	* @return string
	* @access public
	*
	*/
	function adjustBrightness($input, $amount)
	{
		$c = Colour::htmlToRgb($input);
		if ($amount > 0 && $amount <= 1) { // Going up !
			$c['r'] += ((1 - $c['r']) * $amount);
			$c['g'] += ((1 - $c['g']) * $amount);
			$c['b'] += ((1 - $c['b']) * $amount);
		} elseif ($amount < 0 && $amount >= -1) {
			$c['r'] += ($c['r'] * $amount);
			$c['g'] += ($c['g'] * $amount);
			$c['b'] += ($c['b'] * $amount);
		}
		return Colour::rgbToHtml($c);
	}


	/**
	* Adjust contrast on a colour, takes values between -1 and 1
	*
	* @param string	$input				hex string or html colour name
	* @param float	$amount				the amount of brightness (-1 to 1)
	*
	* @return string
	* @access public
	*
	*/
	function adjustContrast($input, $amount)
	{
		$c = Colour::htmlToRgb($input);
		$avg = ($c['r'] + $c['g'] + $c['b']) / 3.0;
		if ($amount < 0) {
			if ($amount < -1) $amount = -1;
			$c['r'] = ($avg * -$amount) + ($c['r'] * (1 + $amount));
			$c['g'] = ($avg * -$amount) + ($c['g'] * (1 + $amount));
			$c['b'] = ($avg * -$amount) + ($c['b'] * (1 + $amount));
		} elseif ($amount > 0) {
			if ($amount >  1) $amount =  1;
			$c['r'] = ( (($c['r'] > $avg) ? 1 : 0) * $amount) + ($c['r'] * (1 - $amount));
			$c['g'] = ( (($c['g'] > $avg) ? 1 : 0) * $amount) + ($c['g'] * (1 - $amount));
			$c['b'] = ( (($c['b'] > $avg) ? 1 : 0) * $amount) + ($c['b'] * (1 - $amount));
		}
		return Colour::rgbToHtml($c);
	}

	/**
	* Mixes two colours in the desired ratio
	*
	* @param string	$a				hex string or html colour name
	* @param string	$b				hex string or html colour name
	* @param float	$ratio			ration between 0 and 1
	*
	* @return string
	* @access public
	*
	*/
	function mix($a, $b, $ratio = 0.5)
	{
		$ratio = max(0,min(1,$ratio));
		$a = Colour::htmlToRgb($a);
		$b = Colour::htmlToRgb($b);
		$c = Array(
			'r' => $a['r'] * (1-$ratio)  +  $b['r'] * $ratio,
			'g' => $a['g'] * (1-$ratio)  +  $b['g'] * $ratio,
			'b' => $a['b'] * (1-$ratio)  +  $b['b'] * $ratio,
		);
		return Colour::rgbToHtml($c);
	}

	/**
	* Rotates a colour around the colour spectrum
	*
	* @param string	$input			hex string or html colour name
	* @param float	$degrees
	*
	* @return string
	* @access public
	*
	*/
	function hueRotate($input,$degrees)
	{
		return Colour::hsvAdjust($input, $degrees, 0, 0);
	}

	/**
	* Adjusts the 'colourfulness' or how non-grey a colour is
	*
	* @param string	$input			hex string or html colour name
	* @param float	$amount			-1 to 1 (-1 = grey)
	*
	* @return string
	* @access public
	*
	*/
	function adjustSaturate($input,$amount)
	{
		return Colour::hsvAdjust($input, 0, $amount, 0);
	}

	/**
	* Adjusts the 'lightness' or value of a colour
	*
	* @param string	$input			hex string or html colour name
	* @param float	$amount
	*
	* @return string
	* @access public
	*
	*/
	function adjustLightness($input,$amount)
	{
		return Colour::hsvAdjust($input, 0, 0, $amount);
	}


	/**
	* Performs all three HSV operations at once with greater accuracy
	*
	* @param string	$input	hex string or html colour name
	* @param float	$h		Hue
	* @param float	$s		Saturation
	* @param float	$v		Value
	*
	* @return string
	* @access public
	*
	*/
	function hsvAdjust($input,$h,$s,$v) 
	{
		$hsv = Colour::htmlToHsv($input);
		if($h != 0) {
			$h = $h % 360.0;
			if($h < 0) $h = 360.0 + $h;
			$hsv['h'] += $h;
			while($hsv['h'] > 360.0) $hsv['h'] -= 360.0;
		}
		if ($s != 0) {
			if ($s > 0) $hsv['s'] += $s * (1 - $hsv['s']);
			else        $hsv['s'] += $s * $hsv['s'];
		}
		if ($v != 0) {
			if ($v > 0) $hsv['v'] += $v * (1 - $hsv['v']);
			else        $hsv['v'] += $v * $hsv['v'];
		}
		return Colour::hsvToHtml($hsv);
	}

	/**
	* Takes two colours and returns a HSV array of values
	*
	* @param string	$a	hex string or html colour name
	* @param string	$b	hex string or html colour name
	*
	* @return string
	* @access public
	* @see colour_remap_new_broken
	*
	*/
	function hsvDifference($a, $b)
	{
		$a    = Colour::htmlToHsv($a);
		$b    = Colour::htmlToHsv($b);
		$r['h'] = $b['h'] - $a['h'];
		if(abs($r['h']) > 180.0) {
			if($r['h'] > 0) $r['h'] -= 360.0;
			else          $r['h'] += 360.0;
		}
		if($b['s'] > $a['s'])	$r['s'] = $b['s'] / (1 - $a['s']) - $a['s'];
		else					$r['s'] = ($b['s'] - $a['s']) / $a['s'];
		if($b['v'] > $a['v'])	$r['v'] = $b['v'] / (1 - $a['v']) - $a['v'];
		else					$r['v'] = ($b['v'] - $a['v']) / $a['v'];
		return $r;
	}


	/**
	* Takes a colour and a remap array.
	* Looks for the 'key' colours and replaces them with the 'value' colours
	* Might work nicer in HSV colour space.
	*
	* @param string	$input_html		hex string or html colour name
	* @param Array	$map			Array('old_colour' => 'new_colour');
	* @param float	$tolerance
	*
	* @return string
	* @access public
	*
	*/
	function remap($input_html,$map,$tolerance = 0.2)
	{
		$tolerance = min(max($tolerance,0),1);
		$input_hsv = Colour::htmlToHsv($input_html);

		$shift           = Array();
		$closeness_total = Array();
		foreach($map as $from => $to) {
			if (!$tolerance) {
				if ($from == $input_html) {
					return $to;
				}
			} else {
				$from     = Colour::htmlToHsv($from);
				$to       = Colour::htmlToHsv($to);
				// The difference between the original colour and the new one
				$map_diff = Array('h'=>$to['h']-$from['h'],'s'=>$to['s']-$from['s'],'v'=>$to['v']-$from['v']);
				// The differnce between the original colour and the input
				$in_diff = Array('h'=>$input_hsv['h']-$from['h'],'s'=>$input_hsv['s']-$from['s'],'v'=>$input_hsv['v']-$from['v']);

				// Work out how 'close' the input is to the from colour, this indicates
				// How much effect this particular mapping should have on it
				$closeness['h'] = pow(1 - abs($in_diff['h'])/360, 16);
				$closeness['s'] = pow(1 - abs($in_diff['s']), 8);
				$closeness['v'] = pow(1 - abs($in_diff['v']), 2);

				$tolerance = min(max($tolerance,0),1);

				$closeness['h'] *= $tolerance;
				$closeness['s'] *= $tolerance;
				$closeness['v'] *= $tolerance;

				$closeness['h'] = max(0,$closeness['h']);
				$closeness['s'] = max(0,$closeness['s']);
				$closeness['v'] = max(0,$closeness['v']);

				// Another modifier ensuring that the hue MUST be close
				$closeness['s'] *= pow($closeness['h'],1);
				$closeness['v'] *= pow($closeness['h'],1);

				// Keep a total of this
				$closeness_total['h'] += $closeness['h'];
				$closeness_total['s'] += $closeness['s'];
				$closeness_total['v'] += $closeness['v'];


				// This is how much the input will be shifted in the output
				// To by divided by the total closeness
				$shift['h'] += $map_diff['h'] * $closeness['h'];
				$shift['s'] += $map_diff['s'] * $closeness['s'];
				$shift['v'] += $map_diff['v'] * $closeness['v'];

			}// end if

		}// end foreach

		// Trim this down again
		if ($tolerance) {
			$input_hsv['h'] += $shift['h'] / $closeness_total['h'];
			$input_hsv['s'] += $shift['s'] / $closeness_total['s'];
			$input_hsv['v'] += $shift['v'] / $closeness_total['v'];
		}

		return Colour::hsvToHtml($input_hsv);

	}// end remap()

}// end class
?>
