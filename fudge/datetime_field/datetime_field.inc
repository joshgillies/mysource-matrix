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
* $Id: datetime_field.inc,v 1.59 2013/04/15 06:35:28 ewang Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

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
* @version $Revision: 1.59 $
* @package Fudge
* @subpackage DateTime_Field
*/
class DateTime_Field
{

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
	* <code>
	* Array(
	*	// datetime elements to show
	* 	'show'			=> Array(['d'],['m'],['y'],['h'],['i'],['s'],['a']),
	*	// style of each of the elements (s = select box, t = textbox)
	* 	'style'			=> Array(
	* 							['d' => '[s|t]',]
	* 							['m' => '[s|t]',]
	* 							['y' => '[s|t]',]
	* 							['h' => '[s|t]',]
	* 							['i' => '[s|t]',]
	* 							['s' => '[s|t]',]
	* 						),
	*	// any extra html attributes to put in the input/select tag
	* 	'extras'			=> Array(
	* 							['d' => '...',]
	* 							['m' => '...',]
	* 							['y' => '...',]
	* 							['h' => '...',]
	* 							['i' => '...',]
	* 							['s' => '...',]
								['a' => '...',]
	* 						),
	*	// datetime elements to allow a null element for
	* 	'null'			=> Array(['d'],['m'],['y'],['h'],['i'],['s']),
	* 	'max'			=> 'YYYY-MM-DD HH:II:SS',
	* 	'min'			=> 'YYYY-MM-DD HH:II:SS',
	* 	'allow_circa'	=> '0',
	*	// see PHP date() fn
	* 	'print_format'	=> 'l, F jS, Y H:i:s',
	* );
	* </code>
	*
	* @var Array()
	*/
	var $parameters='';


	/**
	* Constructor
	*
	* @param int		$prefix			prefix for form elements
	* @param string		&$value			current value of the field
	* @param array		&$parameters	settings for this field (min, max, show etc)
	*
	*/
	function DateTime_Field($prefix, &$value, &$parameters)
	{
		$this->prefix     =& $prefix;
		$this->value      =& $value;
		$this->parameters =& $parameters;

	}//end constructor


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
		$this->value =& $value;

	}//end setValue()


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
		$this->parameters =& $parameters;

	}//end setParameters()


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
		if (isset($this->parameters[$code])) {
			return $this->parameters[$code];
		}
		$res = NULL;
		switch ($code) {
			// default values
			case 'min':
				$res = '0000-00-00 00:00:00';
			break;
			case 'max':
				$res = '9999-12-31 23:59:59';
			break;
		}
		return $res;

	}//end getParam()


	/**
	* Prints the interface for filling in a value
	*
	* @access public
	* @return void
	*/
	function printField()
	{
		$show =& $this->getParam('show');
		$null =& $this->getParam('null');
		if (!is_array($show)) $show = Array();
		if (!is_array($null)) $null = Array();
		$include_js =& $this->getParam('include_js');
		$use_js =& $this->getParam('use_js');
		$print_label =& $this->getParam('print_label');
		// only include JS function if required
		if($use_js && $include_js) {
		?>

		<script type="text/javascript">
		//<![CDATA[
			if (typeof adjustDate != "function") {
				// function to enable + and - keys on a field
				adjustDate = function(input, evt)
				{
					var key = evt.keyCode;
					var value = parseInt(input.value * 1);
					if (key == 107 || key == 109) {
						value += (key == 109)?-1:1;
						if (value != 0) {
							input.value = value;
						}
						event.returnValue = false;
						event.cancelBubble = true;
						return false;
					}
				}
			}
		//]]>
		</script>

		<?php
		}
		echo '<input type="hidden" name="'.$this->prefix.'show" value="'.implode('', $show).'" />';
		$this->prefix .= 'value';
		if ($this->getParam('allow_circa')) {
			combo_box($this->prefix.'[c]', Array('0'=>'','1'=>'circa.'), FALSE, $this->getUnit('c'));
		}
		foreach (Array('d','m','y','h','i','s','a') as $t) {
			$u = $this->getUnit($t);

			if (in_array($t, $show)) {
				$this->inputBox($t, $this->prefix, $u, in_array($t, $null));
			} else if ($t != 'a') {
				echo '<input type="hidden" name="'.$this->prefix.'['.$t.']" value="'.$u.'" />';
			}
			if ($t == 'y') echo ' ';
			if (!$print_label && $t == 'h' && in_array('h', $show) && in_array('i', $show)) {
				echo ':';
			}
			if (!$print_label && $t == 'i' && in_array('i', $show) && in_array('s', $show)) {
				echo ':';
			}
		}

	}//end printField()


	/**
	* Prints an input box for a particular unit
	*
	* @param string		$u			the unit to print
	* @param string		$prefix		prefix for elements
	* @param string		&$value		current value of the unit
	* @param boolean	$allow_null	allow null value (ie blank string)
	*
	* @access public
	* @return void
	*/
	function inputBox($u, $prefix, &$value, $allow_null=FALSE)
	{
		$input_style =& $this->getParam('style');
		$extras =& $this->getParam('extras');
		$increments =& $this->getParam('increment');
		$min_array =& $this->getParam('min');
		$max_array =& $this->getParam('max');
		$print_label =& $this->getParam('print_label');
		$include_js =& $this->getParam('include_js');
		$use_js =& $this->getParam('use_js');

		$min = (int) $this->getUnit($u, $min_array);
		$max = (int) $this->getUnit($u, $max_array);

		$diff = Array();
		foreach (Array('y','m','d','h','i','s') as $diff_unit) {
			$diff[$diff_unit] = $this->getUnit($diff_unit, $max_array) - $this->getUnit($diff_unit, $min_array);
		}

		if ($allow_null && (!strlen($value) || $value == '--' || $value == '----')) {
			$value = '';
		} else if ($u != 'a') {
			$value = (int) $value;
		}

		$increment = 1;

		if (!empty($increments)) $increment = $increments[$u];

		$options = Array();
		if (($u == 'a') || ($input_style[$u] == 's')) { // select box
			if ($allow_null) {
				$options = Array('--' => '');
			}
		}

		// calculate min, max and our selectable options
		switch ($u) {
			case 'y':
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = $i;
			}
			break;
			case 'm':
				// more than 1 years' difference between min and max value
				if (($diff['y']*12) + $diff['m'] >= 12) {
					$min = 1;
					$max = 12;
				// not more than 1 years' difference but rolls over a year boundary
				// (eg. we have a range such as Aug 04..Jan 05)
				} else if ($diff['y'] == 1) {
					// take care of (Jan...$max)
					for ($i = 1; $i <= $max; $i += $increment) {
						$options[$i] = $this->shortMonth($i);
					}
					// now make usual addition take care of ($min...Dec)
					$max = 12;
				}
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = $this->shortMonth($i);
				}
			break;
			case 'd':
				// more than 1 month difference between min and max value
				if (($diff['y'] > 0) || ($diff['m'] > 1) || (($diff['m'] == 1) && ($diff['d'] == 0)) ) {
					$min = 1;
					$max = 31;
				// not more than 1 day difference but rolls over a day boundary
				// (eg. range such as 6d 21h to 7d 05h)
				} else if ($diff['m'] == 1) {
					// take care of (1...$max)
					for ($i = 1; $i <= $max; $i += $increment) {
						$options[$i] = $i;
					}
					// now make usual addition take care of ($min...$end)
					// argh, we need to work out how many days in the rolled-over month!
					$mth = (int) $this->getUnit('m', $max_array);
					$yr = (int) $this->getUnit('y', $max_array);
					$lastday = gmmktime(0, 0, 0, $mth, 0, $yr);	// get the last day of the minimum month
					$max = (int)date('d', $lastday);
				}
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = $i;
				}
			break;
			case 'h':
				// more than 1 day difference between min and max value
				if (($diff['y'] > 0) || ($diff['m'] > 0) || ($diff['d'] > 1) || (($diff['d'] == 1) && ($diff['h'] == 0)) ) {
					if (in_array('a', $this->getParam('show'))) {
						$min = 1;
						$max = 12;
					} else {
						$min = 0;
						$max = 23;
					}
				// not more than 1 day difference but rolls over a day boundary
				// (eg. range such as 6d 21h to 7d 05h)
				} else if ($diff['d'] == 1) {
					// take care of (00...$max)
					for ($i = 0; $i <= $max; $i += $increment) {
						$options[$i] = sprintf('%02d', $i);
					}
					// now make usual addition take care of ($min...23)
					$max = (in_array('a', $this->getParam('show'))) ? 12 : 23;
				}
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = sprintf('%02d', $i);
				}
			break;
			case 'i':
				// more than 1 hours' difference between min and max value
				if (($diff['y'] > 0) || ($diff['m'] > 0) || ($diff['d'] > 0) || ($diff['h'] > 1) || (($diff['h'] == 1) && ($diff['i'] == 0))) {
					$min = 0;
					$max = 59;
				// not more than 1 hours' difference but rolls over a hour boundary
				// (eg. range such as 03:40 to 04:08)
				} else if ($diff['h'] == 1) {
					// take care of (00...$max)
					for ($i = 0; $i <= $max; $i += $increment) {
						$options[$i] = sprintf('%02d', $i);
					}
					// now make usual addition take care of ($min...59)
					$max = 59;
				}
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = sprintf('%02d', $i);
				}
			break;
			case 's':
				// more than 1 minutes' difference between min and max value
				if (($diff['y'] > 0) || ($diff['m'] > 0) || ($diff['d'] > 0) || ($diff['h'] > 0) || ($diff['i'] > 1) || (($diff['i'] == 1) && ($diff['s'] == 0))) {
					$min = 0;
					$max = 59;
				// not more than 1 minutes' difference but rolls over a minute boundary
				// (eg. range such as 03:10:50 to 03:11:20)
				} else if ($diff['i'] == 1) {
					// take care of (00...$max)
					for ($i = 0; $i <= $max; $i += $increment) {
						$options[$i] = sprintf('%02d', $i);
					}
					// now make usual addition take care of ($min...59)
					$max = 59;
				}
				for ($i = (int) $min; $i <= $max; $i += $increment) {
					$options[$i] = sprintf('%02d', $i);
				}
			break;
			case 'a':
				// more than 1 day difference, or less than 1 day but crossing a day boundary, or less than 1 day but crossing midday
				if ((($diff['y'] > 0) || ($diff['m'] > 0) || ($diff['d'] > 1) || (($diff['d'] == 1) && ($diff['h'] == 0)) ) || ($diff['d'] == 1) || (($this->getUnit('h', $min_array) < 12) && ($this->getUnit('h', $max_array) >= 12))) {
					$options = Array('am'=>'am', 'pm'=>'pm');
				// just a morning or afternoon
				} else {
					$options = ($min < 12) ? Array('am'=>'am') : Array('pm'=>'pm');
				}
			break;
		}//end switch
		
		// print label
		if($print_label) {
		    $text = '';
		    switch ($u) {
			case 'y':
			    $text = 'Year';
			    break;
			case 'm':
			    $text = 'Month';
			    break;
			case 'd':
			    $text = 'Day';
			    break;
			case 'h':
			    $text = 'Hour';
			    break;
			case 'i':
			    $text = 'Minute';
			    break;
			case 's':
			    $text = 'Second';
			    break;
			default:
			    break;
		    }
		    label($text, $prefix.'_'.$u);
		    echo ' ';
		}
		
		// print which ever form of input is desired
		if (($u == 'a') || ($input_style[$u] == 's')) {
			if (!isset($extras[$u])) $extras[$u] = '';
			if (!stristr($extras[$u], 'id=')) $extras[$u] .= ' id="'.$prefix.'_'.$u.'"';
			// select box
			combo_box($prefix.'['.$u.']', $options, FALSE, $value, 0, $extras[$u]);
			echo ' ';
			return;
		} else {
			// plain text
			$size = (($u=='y') ? 4 : 2);

			// minutes value should always be 2 digits
			if (($value !== '') && ($u == 'i')) {
				$value = sprintf('%02d', $value);
			}
			if (!isset($extras[$u])) $extras[$u] = '';
			$adjustDate = '';
			if($use_js) {
			    $adjustDate = 'onkeydown="adjustDate(this, event)"';
			}
			if (!stristr($extras[$u], 'id=')) {
				$adjustExtra = $adjustDate.' onfocus="this.select()" id="'.$prefix.'_'.$u.'"';
			} else {
				$adjustExtra = $adjustDate.'onfocus="this.select()"';
			}
			
			int_text_box($prefix.'['.$u.']', $value, FALSE, $size, $min, $max, $size, FALSE, $allow_null, $extras[$u].' '.$adjustExtra, $include_js, $use_js);
			echo ' ';
			return;
		}
		echo '<input type="hidden" name="'.$prefix.'['.$u.']" value="'.$value.'" '.$extras[$u].' />';

	}//end inputBox()


	/**
	* Extracts the value of a particular unit from the full value
	*
	* @param string	$unit	the unit to find the value for
	* @param string	&$value	current value of the field
	*
	* @access public
	* @return string
	*/
	function getUnit($unit, $value='')
	{
		if (empty($value)) {
			$value = $this->value;
			$use_am_pm = in_array('a', $this->getParam('show'));
		} else {
			$use_am_pm = FALSE;
		}

		// separate value into date and time
		$ar_value = explode(' ', $value);
		$date = (isset($ar_value[0])) ? $ar_value[0] : '';
		$time = (isset($ar_value[1])) ? $ar_value[1] : '';
		$c = (isset($ar_value[2])) ? $ar_value[2] : '';

		// separate date into year, month and day
		if (strlen($date) == 10) {
			$y = substr($date, 0, 4);
			$m = substr($date, 5, 2);
			$d = substr($date, 8, 2);
		} else {
			$ar_date = explode('-', $date);
			$y = (isset($ar_date[0])) ? $ar_date[0] : '';
			$m = (isset($ar_date[1])) ? $ar_date[1] : '';
			$d = (isset($ar_date[2])) ? $ar_date[2] : '';
		}

		// separate time into hour, minute, second
		$ar_time = explode(':', $time);
		$a = (isset($ar_time[0]) && ($ar_time[0] >= 12)) ? 'pm' : 'am';
		$h = (isset($ar_time[0])) ? ($use_am_pm ? (((($ar_time[0] % 12) + 11) % 12) + 1) : $ar_time[0]) : '';
		$i = (isset($ar_time[1])) ? $ar_time[1] : '';
		$s = (isset($ar_time[2])) ? $ar_time[2] : '';

		return $$unit;

	}//end getUnit()


	/**
	* Given a month name or number, returns the short name
	*
	* @param string		$m		the month number or name
	* @param boolean	$invert	are you passing in the short name?
	*
	* @access public
	* @return string
	*/
	function shortMonth($m='', $invert=FALSE)
	{
		$months = Array('','Jan','Feb','Mar','Apr','May','Jun','Jul','Aug','Sep','Oct','Nov','Dec');
		if ($invert) {
			$months = array_flip($months);
			$m = ucfirst($m);
		}
		return $months[$m];

	}//end shortMonth()


	/**
	* Given a month name or number, returns the long name
	*
	* @param string		$m		the month number or name
	* @param boolean	$invert	are you passing in the short name?
	*
	* @access public
	* @return string
	*/
	function longMonth($m='', $invert=FALSE)
	{
		$months = Array('','January','February','March','April','May','June','July','August','September','October','November','December');
		if ($invert) {
			$months = array_flip($months);
			$m = ucfirst($m);
		}
		return $months[$m];

	}//end longMonth()


	/**
	* Processes the interface for filling in a value
	*
	* @access public
	* @return boolean
	*/
	function processField()
	{
		if (!isset($_REQUEST[$this->prefix.'value'])) {
			return FALSE;
		}
		$input = $_REQUEST[$this->prefix.'value'];
		$input = $this->unitsArrayToStr($input);
		$validated_value = $this->validateValue($input);
		$this->value = $validated_value;
		return TRUE;

	}//end processField()


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
		if (!is_array($value)) {
			$value = trim($value);
			$l = strlen($value);
			$value = preg_replace('/^c(irc[a]?)?\.?[\s]*/i','', $value);
			$c = FALSE;
			if (($new_l = strlen($value)) != $l) $c = TRUE;
			if (!preg_match("/^\-?[0-9\-]{4}-[0-9\-]{2}-[0-9\-]{2} ([0-9\-]{2}\:?){3}( [01]?)?$/", $value)) {
				// Not "Correct" format
				if (preg_match("/^\-?[0-9]{4}$/", $value)) { // Just a year?
					$value .= '------ --:--:--';
				} else if (preg_match("/^[0-9]+$/", $value)) { // Timestamp?
					$value = date('Y-m-d H:i:s', $value);
				} else {
					$show =& $this->getParam('show');
					// Okay lets try to parse this thing.
					$v = Array('y'=>-1,'m'=>-1,'d'=>-1,'h'=>-1,'i'=>-1,'s'=>-1);
					// Dates
					if (preg_match("/([0-9]{1,2})[\-\/]([0-9]{1,2})[\-\/]([0-9]{2,4})([^0-9].*|)$/", $value, $matches) && ($matches[0] == $value)) {
						// Probably "dd/mm/yyyy" or "mm/dd/yyyy"
						if ($this->getParam('standard') == 'us') {
							$v['d'] = $matches[2]; $v['m'] = $matches[1];
						} else {
							$v['d'] = $matches[1]; $v['m'] = $matches[2];
						}
						$v['y'] = $matches[3];
						if (strlen($v['y']) <= 2) {
							$v['y'] += (($v['y'] < 20) ? 2000 : 1900);
						}
					} else if (preg_match("/\-?([0-9]{2,4})[\-\/]([0-9]{1,2})[\-\/]([0-9]{1,2})([^0-9].*|)$/", $value, $matches) && ($matches[0] == $value)) {
						// Probably a ISO 8601 / ISO 9075 [SQL92/99] date ("yyyy-mm-dd")
						$v['d'] = $matches[3];
						$v['m'] = $matches[2];
						$v['y'] = $matches[1];
						if (strlen($v['y']) <= 2) {
							$v['y'] += (($v['y'] < 20) ? 2000 : 1900);
						}
					} else if (preg_match("/([0-9]{1,2})[\-\/]([0-9]{1,2})([^0-9].*|)$/", $value, $matches)) {
						// Probably dd/mm
						if ($this->getParam('standard') == 'us') {
							$v['d'] = $matches[2]; $v['m'] = $matches[1];
						} else {
							$v['d'] = $matches[1]; $v['m'] = $matches[2];
						}
					} else if (preg_match("/([0-9]{1,2})[\-\/]([0-9]{4})([^0-9].*|)$/", $value, $matches)) {
						// Probably mm/yy
						$v['m'] = $matches[1];
						$v['y'] = $matches[2];
					} else {
						// Maybe a more verbal date.
						if (preg_match("/([0-9]{1,2})?([\s\,]*)(jan(uary)?|feb(ruary)?|mar(ch)?|apr(il)?|may|jun(e)?|jul(y)?|aug(ust)?|sep(tember)?|oct(ober)?|nov(ember)?|dec(ember)?)([\s\,]*)([0-9]{1,2})?([^0-9].*|)$/i", $value, $matches)) {
							// textural month?
							if (strlen($tmp =  $matches[3]) == 3) {
								$v['m'] = $this->shortMonth($tmp,1);
							} else {
								$v['m'] = $this->longMonth($tmp,1);
							}
							// maybe the day?
							if ($matches[1]) {
								$v['d'] = $matches[1];
							} else if ($matches[16]) {
								$v['d'] = $matches[16];
							}
						}
						if (preg_match('/([0-9]{4})([^0-9].*|)$/', $value, $matches)) {
							// four digit year?
							$v['y'] = $matches[1];
						}
						if ($v['d'] == -1 && preg_match('/([0-9]{1,2})(st|nd|rd|th)/i', $value, $matches)) {
							// 1st, 2nd, 3rd, 4th etc
							$v['d'] = $matches[1];
						}

						// Times !
						if (preg_match("/([0-9]{1,2})[\:\.]([0-9]{1,2})[\:\.]([0-9]{1,2})([^0-9].*|)$/", $value, $matches)) {
							$v['h'] = $matches[1];
							$v['i'] = $matches[2];
							$v['s'] = $matches[3];
						} else if (preg_match('/([0-9]{1,2})[\:\.]([0-9]{1,2})([^0-9].*|)$/', $value, $matches)) {
							if (in_array('s', $show) && !in_array('h', $show)) {
								$v['i'] = $matches[1];
								$v['s'] = $matches[2];
							} else {
								$v['h'] = $matches[1];
								$v['i'] = $matches[2];
							}
						}
						if (preg_match('/([0-9]{1,2})([\s]*)(p\.?m\.?|a\.?m\.?)/i', $value, $matches)) {
							if ($v['h'] == -1) $v['h'] = $matches[1];
							if (strtolower($matches[3][0]) == 'p') {
								if ($v['h'] <= 12) $v['h'] += 12;
							} else {
								if ($v['h'] == 12) $v['h'] -= 12;
							}
						}
					}//end else a more verbal form

					$value = $this->unitsArrayToStr($v);

				}//end else not timestamp

			}//end if format matches

			$value = $this->strToUnitsArray($value);
			if ($c) $value['c'] = 1;

		}//end if value not array

		// Now we can assume we have an array of units values to verify
		$min =& $this->getParam('min');
		$max =& $this->getParam('max');
		$value_str = $this->unitsArraytoStr($value);
		if ($value_str != '---------- --:--:--') {
			$value_substr = Array(
								'y'	=> 0,
								'm'	=> 5,
								'd'	=> 8,
								'h'	=> 11,
								'i'	=> 14,
								's'	=> 17,
							);
			foreach (array_keys($value_substr) as $value_part) {
				$substr_length = ($value_part == 'y') ? 4 : 2;
				if ($value[$value_part] == -1) {
					$min = substr_replace($min, str_repeat('-', $substr_length), $value_substr[$value_part], $substr_length);
					$max = substr_replace($max, str_repeat('-', $substr_length), $value_substr[$value_part], $substr_length);
				}
			}

			if (strcmp($value_str, $min) < 0) {
				$value = $this->strToUnitsArray($min);
			} else if (strcmp($value_str, $max) > 0) {
				$value = $this->strToUnitsArray($max);
			}
		}

		// DO CIRCA STUFF HERE ON DOWN
		$validate_day_month = ($value['y'] >= 1970) && ($value['y'] < 2038) && ($value['m'] >= 0) && ($value['d'] >= 0); // PHP limit
		$value = $this->unitsArrayToStr($value);

		// ensures valid
		if ($validate_day_month) {
			$timestamp = mktime(
							(int) substr($value,11,2),
							(int) substr($value,14,2),
							(int) substr($value,17,2),
							(int) substr($value,5,2),
							(int) substr($value,8,2),
							(int) substr($value,0,4)
						);

			$old_value = $value;

			if ((strlen($old_value) == 19) && false === strpos($old_value, '--')) {
				// compare the whole lot
				$value = date('Y-m-d H:i:s', $timestamp);
				if (strcmp($value, $old_value) != 0) {
					trigger_localised_error('SYS0311', E_USER_NOTICE, $old_value);
				}
			} else if (false === strpos($old_value, '----------')) {
				// only compare the date bit
				$value = date('Y-m-d ', $timestamp).substr($old_value, 11);
				if (strcmp(substr($value, 0, 10), substr($old_value, 0, 10)) != 0) {
					trigger_localised_error('SYS0312', E_USER_NOTICE, substr($old_value, 0, 10));
				}
			}

		}

		if (strlen($value) > 20 && !$this->getParam('allow_circa')) {
			$value = substr($value, 20);
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
		$r = Array();
		$r['y'] = ((($i = (int) ($s = substr($value,0,4)))  || ($s{0} != '-' && strlen($s))) ? $i : -1);
		$r['m'] = ((($i = (int) ($s = substr($value,5,2)))  || ($s{0} != '-' && strlen($s))) ? $i : -1);
		$r['d'] = ((($i = (int) ($s = substr($value,8,2)))  || ($s{0} != '-' && strlen($s))) ? $i : -1);
		$r['h'] = ((($i = (int) ($s = substr($value,11,2))) || ($s{0} != '-' && strlen($s))) ? $i : -1);
		$r['i'] = ((($i = (int) ($s = substr($value,14,2))) || ($s{0} != '-' && strlen($s))) ? $i : -1);
		$r['s'] = ((($i = (int) ($s = substr($value,17,2))) || ($s{0} != '-' && strlen($s))) ? $i : -1);
		if ($this->getParam('allow_circa')) {
			$r['c'] = (substr($value, 20, 1) ? 1 : 0);
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
		// convert from 12hr to 24hr if necessary
		if (isset($v['a']) && !empty($v['a'])) {
			$v['h'] = ($v['h'] % 12) + (($v['a'] == 'pm') ? 12 : 0);
		}
		$r = (!isset($v['y']) || ($v['y'] < 0 || !strlen($v['y']) || $v['y']{0} == '-') ? '----' : sprintf('%04d', $v['y'])).'-'
			.(!isset($v['m']) || ($v['m'] < 0 || !strlen($v['m']) || $v['m']{0} == '-') ?  '--'  : sprintf('%02d', $v['m'])).'-'
			.(!isset($v['d']) || ($v['d'] < 0 || !strlen($v['d']) || $v['d']{0} == '-') ?  '--'  : sprintf('%02d', $v['d'])).' '
			.(!isset($v['h']) || ($v['h'] < 0 || !strlen($v['h']) || $v['h']{0} == '-') ?  '--'  : sprintf('%02d', $v['h'])).':'
			.(!isset($v['i']) || ($v['i'] < 0 || !strlen($v['i']) || $v['i']{0} == '-') ?  '--'  : sprintf('%02d', $v['i'])).':'
			.(!isset($v['s']) || ($v['s'] < 0 || !strlen($v['s']) || $v['s']{0} == '-') ?  '--'  : sprintf('%02d', $v['s']));
		if ($this->getParam('allow_circa')) {
			$r .= ' '.(isset($v['c']) && $v['c'] ? '1' : '0');
		}
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
	function format($format=NULL)
	{
		if (is_null($format)) {
			$print_format =& $this->getParam('print_format');
			$format = $print_format;
		}

		// We need to strip certain elements out of the format if we don't have enough
		// information about our datetime.
		// A mapping of which format elements are dependant on which time elements.
		$dep=Array('h' => 'aABcgGhHrU', 'd' => 'cdDIjlOrSUwWzZ', 'm' => 'cDFIlmMnOrtUwWzZ', 'y' => 'cDlLrtUwWYyz', 'i' => 'cirU', 's' => 'crsU');
		$es = '';
		foreach ($v = $this->strToUnitsArray($this->value) as $u => $n) {
			if ($n < 0) {
				if ($u == 'm' || $u == 'd') {
					$$u = 1;
				} else {
					$$u = 0;
				}
				$es .= $dep[$u];
			} else {
				$$u = $n;
			}
		}

		// Certain things we can't work out if the year is before 1970
		$real_y = 0;
		if (!isset($c)) {
			$c = $this->getParam('allow_circa');

		}//end if
		if ($y < 1970 && !stristr($es,'y')) {
			$es .= 'DIlLrUwWzZ';
			$real_y = $y;
			$y      = 1980;
			$format = str_replace('Y', '||||', $format);
			$format = str_replace('y', '~~', $format);
		}
		if ($es) {
			$regexp = "/[\\s]*[^a-zA-Z\\s]*[$es][^a-zA-Z\\|\{4\}\\~\{2\}\\s]*[\\s]*/";
			$format = preg_replace($regexp, ' ', $format);
			if ($real_y) {
				$format = str_replace('||||', $real_y, $format);
				$format = str_replace('~~', $real_y%100, $format);
			}
		}

		// Circa
		$format = str_replace('C', ($c ? '~' : ''), $format);

		$format = date($format,mktime($h, $i, $s, $m, $d, $y));

		// Circa
		$format = str_replace('~', 'circa.', $format);

		return $format;

	}//end format()


	/**
	* Prints fields for inputting the min value
	*
	* @access public
	* @return string
	*/
	function printMinField()
	{
		$p = Array(
				'min'		=> '0000-01-01 00:00:00',
				'max'		=> '9999-12-31 23:59:59',
				'show'		=> Array('y','m','d','h','i','s'),
				'increment'	=> Array(),
				'style'		=> $this->getParam('style'),
			 );
		$p['style']['y'] = 't';
		$min_val =& $this->getParam('min');
		$d = new DateTime_Field($this->prefix.'_min', $min_val, $p);
		$d->printField();

	}//end printMinField()


	/**
	* Processing the min value
	*
	* @access public
	* @return string
	*/
	function processMinField()
	{
		$p = Array(
				'min'		=> '0000-01-01 00:00:00',
				'max'		=> '9999-12-31 23:59:59',
				'show'		=> Array('y','m','d','h','i','s'),
				'increment'	=> Array(),
				'style'		=> $this->getParam('style'),
			 );
		$min =& $this->getParam('min');
		$d = new DateTime_Field($this->prefix.'_min', $min, $p);
		$d->processField();

	}//end processMinField()


	/**
	* Prints fields for inputting the max value
	*
	* @access public
	* @return string
	*/
	function printMaxField()
	{
		$p = Array(
				'min'		=> '0000-01-01 00:00:00',
				'max'		=> '9999-12-31 23:59:59',
				'show'		=> Array('y','m','d','h','i','s'),
				'increment'	=> Array(),
				'style'		=> $this->getParam('style'),
			 );
		$p['style']['y'] = 't';
		$max_val =& $this->getParam('max');
		$d = new DateTime_Field($this->prefix.'_max', $max_val, $p);
		$d->printField();

	}//end printMaxField()


	/**
	* Processing the max value
	*
	* @access public
	* @return string
	*/
	function processMaxField()
	{
		$p = Array(
				'min'		=> '0000-01-01 00:00:00',
				'max'		=> '9999-12-31 23:59:59',
				'show'		=> Array('y','m','d','h','i','s'),
				'increment'	=> Array(),
				'style'		=> $this->getParam('style'),
			 );
		$max =& $this->getParam('max');
		$d = new DateTime_Field($this->prefix.'_max', $max, $p);
		$d->processField();

	}//end processMaxField()


	/**
	* Prints the edit options form
	*
	* @param Array	$show		an array of keys telling which parts of the interface must be printed
	*
	* @access public
	* @return string
	*/
	function printEditOptions($show=Array())
	{
		if (empty($show)) $show = Array('s','n','c','t'); // default set of fields to show

		$us = Array('d'=>'Day','m'=>'Month','y'=>'Year','h'=>'Hour','i'=>'Min','s'=>'Sec');
		$s =& $this->getParam('show');
		$n =& $this->getParam('null');
		$y =& $this->getParam('style');
		$c =& $this->getParam('allow_circa');
		$t =& $this->getParam('standard');
		?>

		<table class="sq-backend-table" style="width: auto;">
			<tr><th>
				<?php
					if (in_array('c', $show)) {
						combo_box($this->prefix.'_allow_circa', Array(''=>'','1'=>'Allow "circa."'), FALSE, $c, 'class="smallprint"');
						echo '<br />';
					}
					if (in_array('t', $show)) {
						combo_box($this->prefix.'_standard', Array('int'=>'Intl. (dd/mm/yyyy)','us'=>'US (mm/dd/yyyy)'), $t, 'class="fineprint"');
					}
				?>
			</th>
			<?php
				foreach ($us as $k=>$v) {
				?>
					<th style="text-align: center;"><?php echo $v; ?></th>
			<?php }//end foreach
			?>
			</tr>

			<?php
					// checking if we should show field selection checkboxes
					if (in_array('s', $show)) {
			?>
			<tr>
				<th>Show</th>
			<?php
				foreach ($us as $k => $v) {
			?>
				<td align="center"><input type="checkbox" name="<?php echo $this->prefix; ?>_show[]" value=<?php echo $k; ?> <?php echo ((in_array($k, $s)) ? 'checked="checked"' : ''); ?>></b>
				</td>
			<?php }//end foreach
			?>
			</tr>
			<?php }//end if in_array
			?>

			<?php
					// checking if we should show blank-allowed checkboxes
					if (in_array('n', $show)) {
			?>
			<tr>
				<th>Allow blank</th>
				<?php
					foreach ($us as $k=>$v) {
				?><td align="center"><input type="checkbox" name="<?php echo $this->prefix; ?>_null[]" value=<?php echo $k; ?> <?php echo ((in_array($k, $n)) ? 'checked="checked"' : ''); ?>></b>
				</td>
			<?php }//end foreach
			?>
			</tr>
			<?php }//end if in_array
			?>

			<tr>
				<th>Select box</th>
			<?php
			foreach ($us as $k=>$v) {
				?>
				<td align="center">
					<input type="radio" name="<?php echo $this->prefix; ?>_style[<?php echo $k; ?>]" value="s" <?php echo (($y[$k]=='s') ? 'checked="checked"' : ''); ?>></b>
				</td>
				<?php
			}
			?>
			</tr>

			<tr>
				<th>Text box</th>
			<?php
			foreach ($us as $k=>$v) {
				?>
				<td align="center">
					<input type="radio" name="<?php echo $this->prefix; ?>_style[<?php echo $k; ?>]" value="t" <?php echo (($y[$k]=='t') ? 'checked="checked"' : ''); ?>></b>
				</td>
				<?php
			}
			?>
			</tr>

			<tr>
				<th style="vertical-align: middle;">Min</th>
				<td colspan="6"><span class="sq-inline-fields-wrapper"><?php $this->printMinField(); ?></span></td>
			</tr>

			<tr>
				<th style="vertical-align: middle;">Max</th>
				<td colspan="6"><span class="sq-inline-fields-wrapper"><?php $this->printMaxField(); ?></span></td>
			</tr>

		</table>

		<?php

	}//end printEditOptions()


	/**
	* Processes the input in edit options form
	*
	* @access public
	* @return string
	*/
	function processEditOptions()
	{
		$s = isset($_REQUEST[$this->prefix.'_show']) ? $_REQUEST[$this->prefix.'_show'] : Array();
		$n = isset($_REQUEST[$this->prefix.'_null']) ? $_REQUEST[$this->prefix.'_null'] : Array();
		$y = $_REQUEST[$this->prefix.'_style'];
		$c = $_REQUEST[$this->prefix.'_allow_circa'];
		$t = isset($_REQUEST[$this->prefix.'_standard']) ? $_REQUEST[$this->prefix.'_standard'] : Array();

		$this->parameters['show']        = $s;
		$this->parameters['null']        = $n;
		$this->parameters['style']       = $y;
		$this->parameters['allow_circa'] = $c;
		$this->parameters['standard']    = $t;

		$this->processMinField();
		$this->processMaxField();

		return TRUE;

	}//end processEditOptions()


	/**
	* Returns TRUE if the datetime field is blank, FALSE otherwise
	*
	* @return boolean
	* @access public
	*/
	function isEmpty()
	{
		return ($this->value == '---------- --:--:--');

	}//end isEmpty()


}//end class

?>
