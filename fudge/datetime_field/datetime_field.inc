<?php
/**
* +--------------------------------------------------------------------+
* | MySource 3 - MySource Matrix                                       |
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
* $Id: datetime_field.inc,v 1.8 2003/11/18 15:42:10 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

/**
* Date/Time Field
*
* Purpose
*     This class is datatype for storing a datetime. The data
*     isn't really important, its just 'YYYY-MM-DD HH:II:SS'
*     its the functionality that counts.
*     It's okay to send a parameters array across with other
*     elements, this thing won't diturb other elements, but
*     it uses: show, style, min, max, null, allow_circa, print_format
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Fudge
* @subpackage DateTime_Field
*/
class DateTime_Field {

	/**
	* The prefix for form fields
	* @var string
	*/
	var $prefix='';

	/**
	* The value of the date/time field
	* @var string
	*/
	var $value='';

	/**
	* Setting for the field (min, max values etc)
	* @var array
	*/
	var $parameters='';


	/**
	* Constructor
	*
	* @param int	$prefix			prefix for form elements
	* @param string	&$value			current value of the field
	* @param array	&$parameters	settings for this field (min, max, show etc)
	*
	*/
	function DateTime_Field($prefix='',&$value,&$parameters)
	{
		$this->prefix     =  $prefix;
		$this->value      = &$value;
		$this->parameters = &$parameters;
	}// end contructor


	/**
	* Sets a reference to the value of this field
	*
	* @param string	&$value	the value to set
	*
	* @access public
	* @return void
	*/
	function setValue(&$value)
	{
		$this->value = &$value;
	}// end setValue()


	/**
	* Sets a reference to the parameters of this field
	*
	* @param array	&$parameters	the parameters to set
	*
	* @access public
	* @return void
	*/
	function setParameters(&$parameters)
	{
		$this->parameters = &$parameters;
	}// end setParameters()


	/**
	* Get a parameter from the parameters array
	*
	* @param string	$code	the name of the parameter
	*
	* @access public
	* @return mixed
	*/
	function &getParam($code='')
	{
		if(isset($this->parameters[$code])) return $this->parameters[$code];
		return;
	}// end getParam()


	/**
	* Prints the interface for filling in a value
	*
	* @access public
	* @return void
	*/
	function printField()
	{
		$show = &$this->getParam('show');
		$null = &$this->getParam('null');
		if (!is_array($show)) $show = Array();
		if (!is_array($null)) $null = Array();
		echo '<input type="hidden" name="'.$this->prefix.'show" value="'.implode('',$show).'" />';
		$this->prefix .= 'value';
		if($this->getParam('allow_circa')) {
			combo_box($this->prefix.'[c]', array('0'=>'','1'=>'circa.'), false, $this->getUnit('c'));
		}
		foreach(array('d','m','y','h','i','s') as $t) {
			$u = $this->getUnit($t);
			if(in_array($t,$show)) {
				$this->inputBox($t,$this->prefix,$u,in_array($t,$null));
			} else {
				echo '<input type="hidden" name="'.$this->prefix.'['.$t.']" value="'.$u.'" />';
			}
			if($t == 'y') echo ' ';
			if($t == 'h' && in_array('h',$show) && in_array('i',$show)) echo ':';
			if($t == 'i' && in_array('i',$show) && in_array('s',$show)) echo ':';
		}
	}// end printField()


	/**
	* Prints an input box for a particular unit
	*
	* @param string		$u			the unit to print
	* @param string		$prefix		prefix for elements
	* @param string		&$value		current value of the unit
	* @param boolean	$allow_null	allow null value (ie --)
	*
	* @access public
	* @return void
	*/
	function inputBox($u='', $prefix='', &$value, $allow_null=false)
	{
		$input_style = &$this->getParam('style');
		$extras = &$this->getParam('extras');
		$increments = &$this->getParam('increment');
		$min = (int) $this->getUnit($u,$this->getParam('min'));
		$max = (int) $this->getUnit($u,$this->getParam('max'));
		if($allow_null && (!strlen($value) || $value == '--')) {
			$value = '--';
		} else {
			$value = (int) $value;
		}

		$increment = $increments[$u];
		if (!$increment) $increment = 1;

		if($input_style[$u] == 's') { // select box
			if($allow_null) $options = array('--' => '--');
			else            $options = array();

			switch($u) {
				case 'y': case 'd':
					for($i = (int) $min; $i <= $max; $i += $increment) $options[$i] = $i;
					break;
				case 'm':
					for($i = (int) $min; $i <= $max; $i += $increment) $options[$i] = $this->shortMonth($i);
					break;
				case 'h': case 'i': case 's':
					for($i = (int) $min; $i <= $max; $i += $increment) $options[$i] = sprintf('%02d',$i);
					break;
			}
			combo_box($prefix.'['.$u.']', $options, false, $value, 0, $extras[$u]);
			return;
		} elseif($input_style[$u] == 't' || 1) { // plain text
			$size = (($u=='y')?4:2);
			if($allow_null) {
				$onchange = "value=parseInt(value);if(isNaN(value)){value='--';}else{value=Math.min($max,Math.max($min,isNaN(value)?0:value));}";
			} else {
				$onchange = "value=parseInt(value);value=Math.min($max,Math.max($min,isNaN(value)?0:value));";
			}

			// merge onChange local values with those coming in from extras
			if (eregi('onchange *="|\'', $extras[$u])) {
				$extra = eregi_replace ("onchange *=(\"|')", 'onChange=\1'.$onchange, $extras[$u]);
				text_box($prefix.'['.$u.']',$value,$size,$size, $extra);
				return;
			} else {
				text_box($prefix.'['.$u.']',$value,$size,$size, $extras[$u]);
				return;
			}
		}
		echo '<input type="hidden" name="'.$prefix.'['.$u.']" value="'.$value.'" '.$extras[$u].' />';
	}// end inputBox()


	/**
	* Extracts the value of a particular unit from the full value
	*
	* @param string	$unit	the unit to find the value for
	* @param string	&$value	current value of the field
	*
	* @access public
	* @return string
	*/
	function getUnit($unit='',$value='')
	{
		if(!strlen($value)) $value = $this->value;
		
		// seperate value into date and time
		$ar_value = explode(' ',$value);
		$date = (isset($ar_value[0])) ? $ar_value[0] : '';
		$time = (isset($ar_value[1])) ? $ar_value[1] : '';
		$c = (isset($ar_value[2])) ? $ar_value[2] : '';

		// seperate date into year, month and day
		$ar_date = explode('-',$date);
		$y = (isset($ar_date[0])) ? $ar_date[0] : '';
		$m = (isset($ar_date[1])) ? $ar_date[1] : '';
		$d = (isset($ar_date[2])) ? $ar_date[2] : '';

		// seperate time into hour, minute, second
		$ar_time = explode(':',$time);
		$h = (isset($ar_time[0])) ? $ar_time[0] : '';
		$i = (isset($ar_time[1])) ? $ar_time[1] : '';
		$s = (isset($ar_time[2])) ? $ar_time[2] : '';

		return $$unit;
	}// end getUnit()


	/**
	* Given a month name or number, returns the short name
	*
	* @param string		$m		the month number or name
	* @param boolean	$invert	are you passing in the short name?
	*
	* @access public
	* @return string
	*/
	function shortMonth($m='',$invert=false)
	{
		$months = array('','Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
		if($invert) {
			$months = array_flip($months);
			$m = ucfirst($m);
		}
		return $months[$m];
	}// end shortMonth()


	/**
	* Given a month name or number, returns the long name
	*
	* @param string		$m		the month number or name
	* @param boolean	$invert	are you passing in the short name?
	*
	* @access public
	* @return string
	*/
	function longMonth($m='',$invert=false)
	{
		$months = array('','January','February','March','April','May','June','July','August','September','October','November','December');
		if($invert) {
			$months = array_flip($months);
			$m = ucfirst($m);
		}
		return $months[$m];
	}// end longMonth()


	/**
	* Processes the interface for filling in a value
	*
	* @access public
	* @return boolean
	*/
	function processField()
	{
		if (!isset($_REQUEST[$this->prefix.'value'])) return false;
		$input = $_REQUEST[$this->prefix.'value'];
		$input = $this->unitsArrayToStr($input);
		$validated_value = $this->validateValue($input);
		if($this->value == $validated_value) return false;
		$this->value = $validated_value;
		return true;
	}// end processField()


	/**
	* Validates the value to ensure it is in the correct format
	*
	* @param string	$value	the value to check
	*
	* @access public
	* @return string
	*/
	function validateValue($value)
	{
		if(!is_array($value)) {
			$value = trim($value);
			$l = strlen($value);
			$value = preg_replace("/^c(irc[a]?)?\.?[\s]*/i",'',$value);
			$c = false;
			if(($new_l = strlen($value)) != $l) {
				$c = true;
			}
			if(!preg_match("/^[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2} ([0-9\-]{2}\:?){3}( [01]?)?$/",$value)) { // Correct format?
				if(preg_match("/^[0-9]{4}$/",$value)) { // Just a year?
					$value .= '------ --:--:--';
				} elseif(preg_match("/^[0-9]+$/",$value)) { // Timestamp?
					$value = date('Y-m-d H:i:s',$value);
				} else {
					$show = &$this->getParam('show');
					// Okay lets try to parse this thing.
					$v = array('y'=>-1,'m'=>-1,'d'=>-1,'h'=>-1,'i'=>-1,'s'=>-1);
					// Dates
					if(preg_match("/([0-9]{1,2})[\-\/]([0-9]{1,2})[\-\/]([0-9]{2,4})([^0-9].*|)$/",$value,$matches)) {
						// Probably "dd/mm/yyyy" or "mm/dd/yyyy"
						if($this->getParam('standard') == 'us') {
							$v['d'] = $matches[2]; $v['m'] = $matches[1];
						} else {
							$v['d'] = $matches[1]; $v['m'] = $matches[2];
						}
						$v['y'] = $matches[3];
						if(strlen($v['y']) <= 2) $v['y'] += (($v['y']<20)?2000:1900);
					} elseif(preg_match("/([0-9]{1,2})[\-\/]([0-9]{1,2})([^0-9].*|)$/",$value,$matches)) {
						// Probably dd/mm
						if($this->getParam('standard') == 'us') {
							$v['d'] = $matches[2]; $v['m'] = $matches[1];
						} else {
							$v['d'] = $matches[1]; $v['m'] = $matches[2];
						}
					} elseif(preg_match("/([0-9]{1,2})[\-\/]([0-9]{4})([^0-9].*|)$/",$value,$matches)) {
						// Probably mm/yy
						$v['m'] = $matches[1];
						$v['y'] = $matches[2];
					} else {
						// Maybe a more verbal date.
						if(preg_match("/([0-9]{1,2})?([\s\,]*)(jan(uary)?|feb(ruary)?|mar(ch)?|apr(il)?|may|jun(e)?|jul(y)?|aug(ust)?|sep(tember)?|oct(ober)?|nov(ember)?|dec(ember)?)([\s\,]*)([0-9]{1,2})?([^0-9].*|)$/i",$value,$matches)) {
							// textural month?
							if(strlen($tmp =  $matches[3]) == 3) {
								$v['m'] = $this->shortMonth($tmp,1);
							} else {
								$v['m'] = $this->longMonth($tmp,1);
							}
							// maybe the day?
							if($matches[1])      $v['d'] = $matches[1];
							elseif($matches[16]) $v['d'] = $matches[16];
						}
						if(preg_match("/([0-9]{4})([^0-9].*|)$/",$value,$matches)) {
							// four digit year?
							$v['y'] = $matches[1];
						}
						if($v['d'] == -1 && preg_match("/([0-9]{1,2})(st|nd|rd|th)/i",$value,$matches)) {
							// 1st, 2nd, 3rd, 4th etc
							$v['d'] = $matches[1];
						}

						// Times !
						if(preg_match("/([0-9]{1,2})[\:\.]([0-9]{1,2})[\:\.]([0-9]{1,2})([^0-9].*|)$/",$value,$matches)) {
							$v['h'] = $matches[1];
							$v['i'] = $matches[2];
							$v['s'] = $matches[3];
						} elseif(preg_match("/([0-9]{1,2})[\:\.]([0-9]{1,2})([^0-9].*|)$/",$value,$matches)) {
							if(in_array('s',$show) && !in_array('h',$show)) {
								$v['i'] = $matches[1];
								$v['s'] = $matches[2];
							} else {
								$v['h'] = $matches[1];
								$v['i'] = $matches[2];
							}
						}
						if(preg_match("/([0-9]{1,2})([\s]*)(p\.?m\.?|a\.?m\.?)/i",$value,$matches)) {
							if($v['h'] == -1) $v['h'] = $matches[1];
							if(strtolower($matches[3][0]) == 'p') {
								if($v['h'] <= 12) $v['h'] += 12;
							} else {
								if($v['h'] == 12) $v['h'] -= 12;
							}
						}

					}
					$value = $this->unitsArrayToStr($v);
				}
			}

			$value = $this->strToUnitsArray($value);
			if($c) $value['c'] = 1;
		}
		// Now we can assume we have an array of units values to verify
		$min = $this->strToUnitsArray($this->getParam('min'));
		$max = $this->strToUnitsArray($this->getParam('max'));
		$us = array('y','m','d','h','i','s');
		foreach($us as $u) {
			if($value[$u] >= 0) {
				$value[$u] = min(max($value[$u],$min[$u]),$max[$u]);
			}
		}
		// DO CIRCA STUFF HERE ON DOWN
		$validate_day_month = $value['y'] > 1901 && $value['y'] < 2038 && $value['m'] >= 0 && $value['d'] >= 0; // PHP limit
		$value = $this->unitsArrayToStr($value);
		// Ensures valid
		if($validate_day_month) {
			$timestamp = mktime(
							(int) substr($value,11,2),
							(int) substr($value,14,2),
							(int) substr($value,17,2),
							(int) substr($value,5,2),
							(int) substr($value,8,2),
							(int) substr($value,0,4)
						);
			$value = date('Y-m-d', $timestamp).substr($value,10);
		}
		if(strlen($value) > 20 && !$this->getParam('allow_circa')) {
			$value = substr($value,20);
		}
		return $value;

	}//end validateValue()


	/**
	* Converts a datetime string "yyyy-mm-dd hh:ii:ss" to an array with an int for each unit. -1 = 'null'
	*
	* @param string	$value	the value to convert in "yyyy-mm-dd hh:ii:ss" format
	*
	* @access public
	* @return array
	*/
	function strToUnitsArray($value)
	{
		$r = array();
		$r['y'] = ((($i = (int) ($s = substr($value,0,4)))  || ($s[0] != '-' && strlen($s))) ? $i : -1);
		$r['m'] = ((($i = (int) ($s = substr($value,5,2)))  || ($s[0] != '-' && strlen($s))) ? $i : -1);
		$r['d'] = ((($i = (int) ($s = substr($value,8,2)))  || ($s[0] != '-' && strlen($s))) ? $i : -1);
		$r['h'] = ((($i = (int) ($s = substr($value,11,2))) || ($s[0] != '-' && strlen($s))) ? $i : -1);
		$r['i'] = ((($i = (int) ($s = substr($value,14,2))) || ($s[0] != '-' && strlen($s))) ? $i : -1);
		$r['s'] = ((($i = (int) ($s = substr($value,17,2))) || ($s[0] != '-' && strlen($s))) ? $i : -1);
		if($this->getParam('allow_circa')) {
			$r['c'] = (substr($value,20,1)?1:0);
		}
		return $r;
	}//end strToUnitsArray()


	/**
	* Converts an array with an int for each unit to a datetime string "yyyy-mm-dd hh:ii:ss"
	*
	* @param array	$value	the array to convert
	*
	* @access public
	* @return string
	*/
	function unitsArrayToStr($v)
	{
		$r = (($v['y'] < 0 || !strlen($v['y']) || $v['y'][0] == '-') ? '----' : sprintf('%04d', $v['y'])).'-'
			.(($v['m'] < 0 || !strlen($v['m']) || $v['m'][0] == '-') ?  '--'  : sprintf('%02d', $v['m'])).'-'
			.(($v['d'] < 0 || !strlen($v['d']) || $v['d'][0] == '-') ?  '--'  : sprintf('%02d', $v['d'])).' '
			.(($v['h'] < 0 || !strlen($v['h']) || $v['h'][0] == '-') ?  '--'  : sprintf('%02d', $v['h'])).':'
			.(($v['i'] < 0 || !strlen($v['i']) || $v['i'][0] == '-') ?  '--'  : sprintf('%02d', $v['i'])).':'
			.(($v['s'] < 0 || !strlen($v['s']) || $v['s'][0] == '-') ?  '--'  : sprintf('%02d', $v['s']));
		if($this->getParam('allow_circa')) $r .= ' '.(isset($v['c']) && $v['c'] ? '1' : '0');
		return $r;
	}//end unitsArrayToStr()


	/**
	* Uses date() to format a value in a given way
	*
	* @param string	$format	the format to print the date in (PHP date format string)
	*
	* @access public
	* @return string
	*/
	function format($format=null)
	{
		if (is_null($format)) $format = $this->getParam('print_format');

		// We need to strip certain elements out of the format if we don't have enough
		// information about our datetime.
		// A mapping of which format elements are dependant on which time elements.
		$dep=array('h' => 'aABgGhHrU', 'd' => 'dDIjlOrSUwWzZ', 'm' => 'DFIlmMnOrtUwWzZ', 'y' => 'DlLrtUwWYyz', 'i' => 'irU', 's' => 'rsU');
		$es = '';
		foreach($v = $this->strToUnitsArray($this->value) as $u => $n) {
			if($n < 0) {
				if($u == 'm' || $u == 'd') $$u = 1;
				else $$u = 0;
				$es .= $dep[$u];
			} else {
				$$u = $n;
			}
		}
		
		// Certain things we can't work out if the year is before 1970
		$real_y = 0;
		$c = 0;
		if($y < 1970 && !stristr($es,'y')) {
			$es .= 'DIlLrUwWzZ';
			$real_y = $y;
			$y      = 1980;
			$format = str_replace('Y','||||',$format);
			$format = str_replace('y','~~',$format);
		}
		if($es) {
			$regexp = "/[\\s]*[^a-zA-Z\\s]*[$es][^a-zA-Z\\|\{4\}\\~\{2\}\\s]*[\\s]*/";
			$format = preg_replace($regexp,' ',$format);
			if($real_y) {
				$format = str_replace('||||',$real_y,$format);
				$format = str_replace('~~',$real_y%100,$format);
			}
		}

		// Circa
		$format = str_replace('c',($c?'|':''),$format);
		$format = str_replace('C',($c?'~':''),$format);

		$format = date($format,mktime($h,$i,$s,$m,$d,$y));

		// Circa
		$format = str_replace('|','c.',$format);
		$format = str_replace('~','circa.',$format);

		return $format;
	}//end format()

}// end class

?>
