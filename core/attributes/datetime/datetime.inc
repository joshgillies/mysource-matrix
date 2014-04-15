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
* $Id: datetime.inc,v 1.46 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Date/Time Asset Attribute
*
* Purpose
*    Asset attribute that stores a single date/time value.
*    This time may be an absolute date, or a relative date, taken from either
*    "now" (the current date AND time) or a certain time "today".
*
* Storage of the field is as follows:
*
* - Single date: YYYY-MM-DD HH:II:SS
*     Stored as a pseudo-ISO 8601 format
*     (Example: "2010-03-03 14:58:30").
*     Where "circa" is allowed, dates marked as "circa" have a "c." prefix.
*     (Example: "c.2010-03-03 14:58:30").
*
* - Period from "now" (current date/time): +Dx
*     Stored in an abbreviated version of format expected by strtotime(),
*     Where "D" is the duration, and "x" is the date time period,
*     as used in PHP's date() function.
*     Minutes to years accepted.
*     (Examples: "+0h" is "now"; "-90i" is 90 minutes prior to now;
*      "+2w" is 2 weeks later than now)
*
* - Period from "today" (current date) at specified time: HH:II:SS +Dx
*     Stored as time + period.
*     Days to years accepted (minutes and hours don't exactly make much sense)
*     (Examples: "00:00:00 +0d" means midnight "today"; "20:00:00 -1w" means
*      8:00pm, this day one week ago)
*
* - Null date: "---------- --:--:--"
*     A "null date" is treated as a "single date".
*
* <!-- Sample Edit Interface XML Node -->
*    <datetime min="1970-01-01 00:00:00" max="2030-12-31 23:59:59" allow_circa="1" allow_period="1" allow_period_today="1" use_picker="1">
*      <show>
*        <y style="t" allow_null="1" />
*        <m style="s" allow_null="1" />
*        <d style="s" allow_null="1" />
*        <h style="s" allow_null="1" />
*        <i style="s" allow_null="1" />
*        <s style="s" allow_null="1" />
*      </show>
*    </datetime>
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.46 $
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_DateTime extends Asset_Attribute
{


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_DateTime($attributeid=0, $value=NULL)
	{
		$this->Asset_Attribute($attributeid, $value);

		// default values
		$this->_edit_params['allow_null']         = TRUE;
		$this->_edit_params['max']                = '2030-12-31 23:59:59';
		$this->_edit_params['min']                = '1970-01-01 00:00:00';
		$this->_edit_params['allow_circa']        = '0';
		$this->_edit_params['print_format']       = 'l, F jS, Y H:i:s';
		$this->_edit_params['allow_period']       = FALSE;
		$this->_edit_params['allow_period_today'] = FALSE;
		$this->_edit_params['use_picker']         = FALSE;

		// default date format and display style.
		$this->_edit_params['show']			= Array('y', 'm', 'd', 'h', 'i', 's');
		$this->_edit_params['style']		= Array(
												'y'	=> 's',
												'm'	=> 's',
												'd'	=> 's',
												'h'	=> 's',
												'i'	=> 's',
												's'	=> 's',
											  );
		$this->_edit_params['extras']		= Array(
												'y'	=> '',
												'm'	=> '',
												'd'	=> '',
												'h'	=> '',
												'i'	=> '',
												's'	=> '',
											  );
		$this->_edit_params['null'] 		= Array();

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object	&$node	XML_Tree_Node object.
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return FALSE;

		if (!empty($node->attributes()->min)) {
			$this->_edit_params['min'] = (string)$node->attributes()->min;
		}
		if (!empty($node->attributes()->max)) {
			$this->_edit_params['max'] = (string)$node->attributes()->max;
		}
		if (!empty($node->attributes()->allow_circa)) {
			$this->_edit_params['allow_circa'] = (string)$node->attributes()->allow_circa;
		}
		if (!empty($node->attributes()->print_format)) {
			$this->_edit_params['print_format'] = (string)$node->attributes()->print_format;
		}

		// what to print and how to show it (textbox or select box)
		// need to do some more intense looking around for these options :)
		$this->_edit_params['show']  = Array();
		$this->_edit_params['style'] = Array();
		foreach ($node->children() as $c => $child) {
			if ($child->getName() == 'show') {
				foreach ($child->children() as $i => $format) {
					$this->_edit_params['show'][] = $format->getName();
					$this->_edit_params['style'][(string) $format->getName()] = (string)$format->attributes()->style;
					if (isset($format->attributes()->allow_null) && (string)$format->attributes()->allow_null) {
						$this->_edit_params['null'][] = $format->getName();
					}
				}
				break;
			}
		}

		if (isset($node->attributes()->allow_period) && (string)$node->attributes()->allow_period) {
			$this->_edit_params['allow_period'] = TRUE;
		}

		if (isset($node->attributes()->use_picker) && (string)$node->attributes()->use_picker) {
			$this->_edit_params['use_picker'] = TRUE;
		}

		if (isset($node->attributes()->allow_period_today) && (string)$node->attributes()->allow_period_today) {
			$this->_edit_params['allow_period_today'] = TRUE;
		}

		// if nothing was found, define some defaults of our own
		if (empty($this->_edit_params['show'])) {
			$this->_edit_params['show'] = Array('y','m','d','h','i','s');
			foreach ($this->_edit_params['show'] as $u) {
				$this->_edit_params['style'][$u] = 's';
			}
		}

		return TRUE;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @return void
	* @access public
	*/
	function paint($prefix, $read_only=FALSE)
	{
		$date_option = NULL;

		$period_options = $this->_getPeriodOptions();
		$age_options = $this->_getAgeOptions();

		$period_num  = '1';
		$period_type = 'd';
		$period_age  = '-';
		$base_time   = '00:00:00';

		$matches = Array();

		$circa = NULL;
		if (substr($this->value, 0, 2) === 'c.') {
			$circa = TRUE;
			$date_value = substr($this->value, 2);
		} else {
			$circa = FALSE;
			$date_value = $this->value;
		}

		if ((empty($date_value) === TRUE) || (is_iso8601($date_value) === TRUE)) {
			// Single date
			// To be stored in pseudo-ISO 8601 format (yyyy-mm-dd hh:ii:ss)
			$date_option = 'single_date';

		} else {
			// Test for time period, from current date/time
			// eg. "+1m", "-8w", etc
			$found = preg_match('/^([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $date_value, $matches);

			if ($found === 1) {
				// All good
				$date_option = 'date_period';
				list(, $period_age, $period_num, $period_type) = $matches;

			} else {
				// What about a time period from "today"?
				// eg. "12:30:00 -10d"
				$found = preg_match('/^([0-9]{2}):([0-9]{2}):([0-9]{2}) ([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $date_value, $matches);

				if ($found === 1) {
					// All good
					list($period_age, $period_num, $period_type) = array_slice($matches, 4, 3);
					$date_option = 'date_period_today';

				} else {
					// Not an option!
					$date_value = '0000-00-00 00:00:00';
					$date_value = str_replace('0', '-', $date_value);
					$date_option = 'single_date';

				}//end if date period (from "today")


			}//end if date period (from "now")

		}//end if single date

		// Calculate the default values to shown in the other date types
		switch ($date_option) {
			case 'single_date':
				$date_field_value = $date_value;

				if (empty($date_field_value) === TRUE) {
					// Empty
					$date_field_value = ts_iso8601(time());
					$date_period_value = '';

				} else if ($date_field_value !== '---------- --:--:--') {
					// Not null date
					// Calculate a default time period to put into the "date period"
					$timestamp = strtotime(str_replace('--', '0', $date_field_value));

					if ($timestamp < time()) {
						$format = easy_time_total(time() - $timestamp, FALSE, TRUE);
						$period_age = '-';

					} else {
						$format = easy_time_total($timestamp - time(), FALSE, TRUE);
						$period_age = '+';

					}

					if (empty($format) === FALSE) {
						if ($format['d'] > 0) {
							$period_num = $format['d'];
							$period_type = 'd';

						} else if ($format['h'] > 0) {
							$period_num = $format['h'];
							$period_type = 'h';

						} else if ($format['m'] > 0) {
							$period_num = $format['m'];
							$period_type = 'i';

						} else if ($format['s'] > 0) {
							$period_num = $format['s'];
							$period_type = 's';

						}

					}

					$date_period_value = $period_age.$period_num.$period_type;
				} else {
					$date_period_value = '';
				}

			break;

			case 'date_period':
				$date_period_value = $period_age.$period_num.$period_type;
				$date_period = $period_age.$period_num.' '.$period_options[$period_type];
				$date_field_value = ts_iso8601(strtotime($date_period));
				$base_time = '00:00:00';
			break;

			case 'date_period_today':
				$date_period_value = $period_age.$period_num.$period_type;
				$date_period = $period_age.$period_num.' '.$period_options[$period_type];
				$base_time = $matches[1].':'.$matches[2].':'.$matches[3];
				$date_field_value = ts_iso8601(strtotime($base_time.' '.$date_period));
			break;

		}//end switch on date type

		if (!empty($this->_params['print_format'])) {
			$this->_edit_params['print_format'] = $this->_params['print_format'];
		}

		if ($read_only === TRUE) {
			switch ($date_option) {
				case 'single_date':
					if ($this->_edit_params['allow_circa'] === '1') {
						if ($circa === TRUE) {
							$date_field_value .= ' 1';
						} else {
							$date_field_value .= ' 0';
						}
					}
					$field = new Datetime_Field($prefix, $date_field_value, $this->_edit_params);
					echo $field->format();
				break;

				case 'date_period':
					if ((int)$period_num === 0) {
						echo translate('attribute_datetime_period_age_0', $period_num, $period_options[$period_type]);
					} else {
						echo translate('attribute_datetime_period_age_'.$period_age, $period_num, $period_options[$period_type]);
					}
				break;

				case 'date_period_today':
					if ((int)$period_num === 0) {
						echo translate('attribute_datetime_today_age_0', $period_num, $period_options[$period_type], $base_time);
					} else {
						echo translate('attribute_datetime_today_age_'.$period_age, $period_num, $period_options[$period_type], $base_time);
					}
				break;

			}//end switch on date option

		} else {
			$options = Array(
						'single_date'		=> translate('attribute_datetime_option_single_date'),
						'date_period'		=> translate('attribute_datetime_option_date_period'),
						'date_period_today'	=> translate('attribute_datetime_option_date_period_today'),
					   );

			if ((boolean)$this->_edit_params['allow_period'] === FALSE) {
				unset($options['date_period']);
			}
			if ((boolean)$this->_edit_params['allow_period_today'] === FALSE) {
				unset($options['date_period_today']);
			}

			$edit_fields = Array();
			foreach ($options as $option_key => $option_name) {
				$edit_fields[$option_key] = $this->_getDateOptionEditInterface($prefix, $option_key, $date_field_value, $date_period_value, $circa);
			}

			if (count($edit_fields) > 1) {
				?>
				<fieldset>
					<legend>
					<select name="<?php echo $prefix ?>_period_select" id="<?php echo $prefix ?>_period_select" onchange="items = document.getElementsByClassName('sq-datetime-field-date-option-<?php echo $prefix; ?>'); for (var x = 0; x < items.length; x++) {items[x].style.display = (items[x].id == ('<?php echo $prefix; ?>_field_' + this.value)) ? 'block' : 'none' };">
							<?php foreach ($options as $option_key => $option_name) {
								?><option value="<?php echo $option_key ?>"<?php echo ($option_key === $date_option) ? ' selected="selected"' : ''; ?>><?php echo $option_name ?></option>
							<?php } ?>
						</select>
					</legend>
				<?php
			}

			foreach ($options as $option_key => $option_name) {
					?><div id="<?php echo $prefix; ?>_field_<?php echo $option_key ?>" class="sq-datetime-field-date-option-<?php echo $prefix; ?>" style="display: <?php echo ($option_key === $date_option) ? 'block' : 'none'; ?>;">
						<p style="margin-bottom: 0;"><?php echo $edit_fields[$option_key]; ?></p>
						<div class="sq-backend-smallprint"><?php echo translate('attribute_datetime_select_'.$option_key); ?></div>
					</div><?php
			}//end foreach

			if (count($edit_fields) > 1) {
				?>
				</fieldset>
				<?php
			}

		}//end if read-only

	}//end paint()


	/**
	* Processes the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$date_option = array_get_index($_REQUEST, $prefix.'_period_select', 'single_date');
		$field_value = $this->value;

		switch ($date_option) {
			case 'single_date':
				$this->processed = FALSE;

				// If the current value cannot be used as a single date and we
				// can't process what we have, reset to a null date
				if (is_iso8601($field_value) === FALSE) {
					$field_value = '---------- --:--:--';
				}

				$field = new Datetime_Field($prefix, $field_value, $this->_edit_params);
				if ($field->processField() === TRUE) {
					if ($this->_edit_params['allow_circa'] === '1') {
						$value_bits = explode(' ', $field_value);
						if ($value_bits[2] === '0') {
							$field_value = $value_bits[0].' '.$value_bits[1];
						} else {
							$field_value = 'c.'.$value_bits[0].' '.$value_bits[1];
						}
					}

					// Attribute has changed
					if ($this->value !== $field_value) {
						$ok = $this->setValue($field_value);
						if ($ok === TRUE) {
							// Processed OK
							$this->processed = TRUE;
						}
					}
				}
			break;

			case 'date_period':
				$this->processed = FALSE;

				// If not set, default to "1 day in the past"
				$period_num  = (int) array_get_index($_REQUEST, $prefix.'period_num', 1);
				$period_type = (string) array_get_index($_REQUEST, $prefix.'_period_type', 'd');
				$period_age  = (string) array_get_index($_REQUEST, $prefix.'_period_age', '-');

				// Are the periods/ages passed to us valid?
				$period_options = $this->_getPeriodOptions();
				$age_options = $this->_getAgeOptions();

				if (isset($period_options[$period_type]) === FALSE) {
					$period_type = 'd';
				}
				if (isset($age_options[$period_age]) === FALSE) {
					$period_age = '-';
				}

				$field_value = $period_age.$period_num.$period_type;

				if ($this->value !== $field_value) {
					$ok = $this->setValue($field_value);
					if ($ok === TRUE) {
						// Processed OK
						$this->processed = TRUE;
					}
				}

			break;

			case 'date_period_today':
				$this->processed = FALSE;

				// If not set, default to "Today at midnight"
				$period_num  = (int) array_get_index($_REQUEST, $prefix.'_today_period_num', 0);
				$period_type = (string) array_get_index($_REQUEST, $prefix.'_today_period_type', 'd');
				$period_age  = (string) array_get_index($_REQUEST, $prefix.'_today_period_age', '-');
				$base_time = '00:00:00';

				// Are the periods/ages passed to us valid? (Certain periods
				// can't be used.)
				$period_options = $this->_getPeriodOptions();
				$age_options = $this->_getAgeOptions();
				unset($period_options['h']);
				unset($period_options['i']);

				if (isset($period_options[$period_type]) === FALSE) {
					$period_type = 'd';
				}
				if (isset($age_options[$period_age]) === FALSE) {
					$period_age = '-';
				}

				// Now grab the base time from the datetime field, and if that
				// is good, add the time component of it to the period
				$edit_params = $this->_edit_params;
				$edit_params['show'] = Array('h', 'i', 's');
				$field = new Datetime_Field($prefix.'_today_time', $base_time, $edit_params);

				if ($field->processField() === TRUE) {
					$base_time = iso8601_time_component($base_time);
					$field_value = $base_time.' '.$period_age.$period_num.$period_type;

					if ($this->value !== $field_value) {
						$ok = $this->setValue($field_value);
						if ($ok === TRUE) {
							// Processed OK
							$this->processed = TRUE;
						}
					}
				}

			break;

		}//end switch

		return;

	}//end process()


	/**
	* Get the current value as a unix timestamp
	*
	* @return int
	* @access public
	*/
	function getTimestamp()
	{
		$date_field_value = NULL;

		if (is_iso8601($this->value) === TRUE) {
			// Single date
			$date_field_value = $this->value;

		} else if (substr($this->value, 0, 2) === 'c.') {
			// Single date with circa
			if (is_iso8601(substr($this->value, 2)) === TRUE) {
				$date_field_value = substr($this->value, 2);
			}

		} else {
			// Are the periods/ages passed to us valid?
			$period_options = $this->_getPeriodOptions();
			$age_options = $this->_getAgeOptions();

			$found = preg_match('/^([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $this->value, $matches);
			if ($found === 1) {
				// Date period from "now"
				list(, $period_age, $period_num, $period_type) = $matches;
				$date_field_value = date('Y-m-d H:i:s', strtotime($period_age.$period_num.' '.$period_options[$period_type]));

			} else {
				$found = preg_match('/^([0-9]{2}:[0-9]{2}:[0-9]{2}) ([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $this->value, $matches);
				if ($found === 1) {
					// Date period from "today" at certain time
					list(, $base_time, $period_age, $period_num, $period_type) = $matches;
					$date_field_value = date('Y-m-d H:i:s', strtotime($base_time.' '.$period_age.$period_num.' '.$period_options[$period_type]));
				} else {
					$date_field_value = NULL;
				}
			}

		}

		$timestamp = NULL;

		if ($date_field_value === NULL) {
			$timestamp = -1;
		} else {
			$field = new Datetime_Field('blah', $date_field_value, $this->_edit_params);
			$units = $field->strToUnitsArray($date_field_value);
			foreach (Array('y', 'm', 'd') as $u) {
				if ($units[$u] == -1) {
					$timestamp = -1;
				}
			}

			if ($timestamp !== -1) {
				// now we know that we can formulate a date
				$strtotime = $units['m'].'/'.$units['d'].'/'.$units['y'];

				$dotime = TRUE;
				foreach (Array('h', 'i', 's') as $u) {
					if ($units[$u] == -1) $dotime = FALSE;
				}
				if ($dotime) {
					$strtotime .= ' '.$units['h'].':'.$units['i'].':'.$units['s'];
				}

				$timestamp = strtotime($strtotime);
			}
		}

		return $timestamp;

	}//end getTimestamp()


	/**
	* Get the current value as an ISO8601 formatted date
	*
	* @return string
	* @access public
	*/
	function getISO8601()
	{
		if (is_iso8601($this->value) === TRUE) {
			return $this->value;
		} else {
			$time_zone_designater = date('O');
			$tzd_iso8601 = substr($time_zone_designater, 0, 3).':'.substr($time_zone_designater, -2);

			return date('Y-m-d\TH:i:s', $this->getTimestamp()).$tzd_iso8601;
		}

	}//end getISO8601()


	/**
	* Ensure the attribute being set is a valid date/time
	*
	* @param mixed	&$value	value to be validated
	*
	* @return boolean
	* @access public
	*/
	function validateValue(&$value)
	{
		// specific check for null
		if (!empty($this->_params['allow_null']) && ($value == '---------- --:--:--' || $value == '')) {
			return TRUE;
		}

		// See if we have a 'circa' notation (for single date only)
		// We support 'c.', 'circ.', 'circa.' - with and without dot,
		// with and without trailing space before date itself.
		$found = preg_match('/^(c(irc(a)?)?[\.]?[ ]?)?(.*)$/', $value, $matches);
		if ($found) {
			$date_value = $matches[4];
			$circa = FALSE;
			if (empty($matches[1]) === FALSE) {
				$circa = TRUE;
			}
		} else {
			$date_value = $value;
		}

		if (is_iso8601($date_value) === TRUE) {
			$ref = '';
			$field = new Datetime_Field('blah', $ref, $this->_edit_params);
			$value = $field->validateValue($value);
			if ($this->_edit_params['allow_circa'] === '1') {
				$value_bits = explode(' ', $value);
				if ($value_bits[2] === '0') {
					$value = $value_bits[0].' '.$value_bits[1];
				} else {
					$value = 'c.'.$value_bits[0].' '.$value_bits[1];
				}
			}

		} else {
			if (!empty($date_value) && !preg_match('|([0-9]{2}:[0-9]{2}:[0-9]{2} )?[\-\+][0-9]+['.implode('', array_keys($this->_getPeriodOptions())).']|', $date_value)) {
				return FALSE;
			}
		}

		return TRUE;

	}//end validateValue()


	/**
	* Return valid period type options for the datetime field
	*
	* @return array
	* @access private
	*/
	function _getPeriodOptions()
	{
		return Array(
				'i'	=> translate('minutes'),
				'h'	=> translate('hours'),
				'd'	=> translate('days'),
				'w'	=> translate('weeks'),
				'm'	=> translate('months'),
				'y'	=> translate('years'),
			   );

	}//end _getPeriodOptions()


	/**
	* Return valid period age options for the datetime field
	*
	* @return array
	* @access private
	*/
	function _getAgeOptions()
	{
		return Array(
				'-'	=> 'in the past',
				'+'	=> 'in the future',
			   );

	}//end _getAgeOptions()


	/**
	* Return valid "Today" period age options
	*
	* @return array
	* @access private
	*/
	function _getTodayAgeOptions()
	{
		return Array(
				'-'	=> 'before today',
				'+'	=> 'after today',
			   );

	}//end _getTodayAgeOptions()


	/**
	* Returns the content of this attribute as it should be shown in a keyword replacement
	*
	* @param string	$format	Format suffix
	*
	* @return void
	* @access public
	*/
	function getKeywordValue($format='')
	{
		if ($format !== '') {
			// Get the current date/time, whether fixed or calculated from period
			$ts = $this->getTimestamp();

			if ($ts !== -1) {
				$format_value = format_date($ts, $format);
				if ($format_value) {
					return $format_value;
				}
			}
		}
		return parent::getKeywordValue($format);

	}//end getKeywordValue()


	/**
	* Returns the edit interface for each date option type
	*
	* @param string		$prefix			Prefix for form elements
	* @param string		$option_name	Date option type to paint
	* @param string		$default_date	Default fixed date
	* @param string		$default_period	Default date period setting
	* @param boolean	$circa			NULL = circa not allowed, otherwise TRUE/FALSE
	*									depending on circa setting
	*
	* @return string
	*/
	protected function _getDateOptionEditInterface($prefix, $option_name, $default_date=NULL, $default_period=NULL, $circa=NULL)
	{
		ob_start();

		switch ($option_name) {
			case 'single_date':
				if ($this->_edit_params['allow_circa'] === '1') {
					if ($circa === TRUE) {
						$default_date .= ' 1';
					} else {
						$default_date .= ' 0';
					}
				}

				$field = new Datetime_Field($prefix, $default_date, $this->_edit_params);
				$field->printField();

				if ($this->_edit_params['use_picker']) {
					// print the JS calendar popup date selecta
					require_once SQ_LIB_PATH.'/js_calendar/js_calendar.inc';

					$calendar = new JS_Calendar();
					$calendar->changeSetting('onDayClick', 'datetime_set_date');
					$calendar->paint($prefix, $prefix.'_cal_picker', TRUE);
				}
			break;

			case 'date_period':
				$period_options = $this->_getPeriodOptions();
				$age_options = $this->_getAgeOptions();

				$found = preg_match('/^([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', $default_period, $matches);
				if ($found === 1) {
					list(, $period_age, $period_num, $period_type) = $matches;
				} else {
					list($period_age, $period_num, $period_type) = Array('-', '', 'd');
				}

				ob_start();
					int_text_box($prefix.'period_num', $period_num, FALSE, 3);
				$cb_period_num = ob_get_clean();

				ob_start();
					combo_box($prefix.'_period_type', $period_options, FALSE, $period_type);
				$cb_period_type = ob_get_clean();

				ob_start();
					combo_box($prefix.'_period_age', $age_options, FALSE, $period_age);
				$cb_period_age = ob_get_clean();

				echo translate('attribute_datetime_date_period_edit_format', $cb_period_num, $cb_period_type, $cb_period_age);

			break;

			case 'date_period_today':
				$period_options = $this->_getPeriodOptions();
				$age_options = $this->_getTodayAgeOptions();

				$found = preg_match('/^[0-9]{2}:[0-9]{2}:[0-9]{2} ([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])$/', date('H:i:s', strtotime($default_date)).' '.$default_period, $matches);
				if ($found === 1) {
					list(, $period_age, $period_num, $period_type) = $matches;
				} else {
					list($period_age, $period_num, $period_type) = Array('-', '', 'd');
				}

				// If we have inherited an unsupported period type (ie. switching
				// from date period using hours or minutes), convert it into days
				// and round down.
				if ($period_type === 'i') {
					$period_num = floor($period_num / 1440);
					$period_type = 'd';
				} else if ($period_type === 'h') {
					$period_num = floor($period_num / 24);
					$period_type = 'd';
				}

				ob_start();
					int_text_box($prefix.'_today_period_num', $period_num, FALSE, 3);
				$cb_period_num = ob_get_clean();

				ob_start();
					// No point having options smaller than "days" here
					unset($period_options['i']);
					unset($period_options['h']);
					combo_box($prefix.'_today_period_type', $period_options, FALSE, $period_type);
				$cb_period_type = ob_get_clean();

				ob_start();
					combo_box($prefix.'_today_period_age', $age_options, FALSE, $period_age);
				$cb_period_age = ob_get_clean();

				ob_start();
					$edit_params = $this->_edit_params;
					$edit_params['show'] = Array('h', 'i', 's');

					$field = new Datetime_Field($prefix.'_today_time', $default_date, $edit_params);
					$field->printField();
				$datetime_field_output = ob_get_clean();

				echo translate('attribute_datetime_today_period_edit_format', $cb_period_num, $cb_period_type, $cb_period_age, $datetime_field_output);

			break;

		}//end switch

		$output = ob_get_clean();

		// If empty, it must have been an invalid option, so return FALSE
		if (empty($output) === TRUE) {
			$output = FALSE;
		}

		return $output;

	}//end _getDateOptionEditInterface()


}//end class

?>
