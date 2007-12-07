<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: datetime.inc,v 1.43 2007/12/07 05:58:22 bshkara Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_attribute.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';

/**
* Date/Time Asset Attribute
*
*
*
*
* <!-- Sample Edit Interface XML Node -->
*    <datetime min="1970-01-01 00:00:00" max="2030-12-31 23:59:59" allow_circa="1" allow_period="1" use_picker="1">
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
* @version $Revision: 1.43 $
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
		$this->_edit_params['allow_null']   = TRUE;
		$this->_edit_params['max']          = '2030-12-31 23:59:59';
		$this->_edit_params['min']          = '1970-01-01 00:00:00';
		$this->_edit_params['allow_circa']  = '0';
		$this->_edit_params['print_format'] = 'l, F jS, Y H:i:s';
		$this->_edit_params['allow_period'] = FALSE;
		$this->_edit_params['use_picker']   = FALSE;
		$this->_edit_params['show']			= Array();

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
	{// TODO TOF TEST IMPORTANT
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
		$using_single_date = is_iso8601($this->value) || empty($this->value);

		$period_num = '1';
		$period_type = 'd';
		$period_age = '-';

		$period_options = $this->_getPeriodOptions();
		$age_options = $this->_getAgeOptions();

		if ($using_single_date) {
			$date_field_value = $this->value;

			if (($date_field_value != '---------- --:--:--') && (!empty($date_field_value))) {

				// get the current value as a timestamp
				// NOTE: we replace -- with ZERO because strtotime needs a full iso8601 date
				// to get the timestamp - not just part of one
				$timestamp = strtotime(str_replace('--', '0', $this->value));

				if ($timestamp < time()) {
					$format = easy_time_total(time() - $timestamp, FALSE, TRUE);
					if (!empty($format)) {
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
				}
			} else if (empty($date_field_value)) {
				$date_field_value = date('Y-m-d');
			}

		} else {
			$matches = Array();
			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			$date_field_value = date('Y-m-d H:i:s', strtotime($period_age.$period_num.' '.$period_options[$period_type]));
		}

		if (!empty($this->_params['print_format'])) {
			$this->_edit_params['print_format'] = $this->_params['print_format'];
		}
		$field = new Datetime_Field($prefix, $date_field_value, $this->_edit_params);

		if ($using_single_date) {
			if ($read_only) {
				echo $field->format();
				return;
			}
			$start_text = translate('select_single_period');
		} else {
			if ($read_only) {
				echo $period_num.' '.$period_options[$period_type].' '.$age_options[$period_age];
				return;
			}
			$start_text = translate('select_date_period_config');
		}

		if ($this->_edit_params['allow_period']) {
			?>
			<fieldset>
				<legend>
					<span id="<?php echo $prefix; ?>_selecta"><?php echo $start_text; ?></span>
					[
					<a class="sq-backend-legend" href="#"
					style="text-decoration: none;"
					onclick="sq_toggle_double_div('<?php echo $prefix; ?>_date_field', '<?php echo $prefix; ?>_period_field', '<?php echo $prefix; ?>_selecta', 'Select Single Date', 'Select Date Period'); set_hidden_field('<?php echo $prefix; ?>_setting_single_date', document.getElementById('<?php echo $prefix; ?>_date_field').style.display == 'block'); return false;"><?php echo strtolower(translate('change')); ?></a>
					]
				</legend>
				<div id="<?php echo $prefix; ?>_date_field" style="display: <?php echo ($using_single_date) ? 'block' : 'none'; ?>;">
					<?php
						$field->printField();

						if ($this->_edit_params['use_picker']) {
							// print the JS calendar popup date selecta
							require_once SQ_LIB_PATH.'/js_calendar/js_calendar.inc';

							$calendar = new JS_Calendar();
							$calendar->changeSetting('onDayClick', 'datetime_set_date');
							$calendar->paint($prefix, $prefix.'_cal_picker', TRUE);
						}
					?>
					<br /><br /><i><?php echo translate('select_single_date'); ?></i>
				</div>
				<div id="<?php echo $prefix; ?>_period_field" style="display: <?php echo ($using_single_date) ? 'none' : 'block'; ?>;">
					<?php
					text_box($prefix.'period_num', $period_num, 3);
					echo '&nbsp;';
					combo_box($prefix.'_period_type', $period_options, FALSE, $period_type);
					echo '&nbsp;';
					combo_box($prefix.'_period_age', $age_options, FALSE, $period_age);
					?>
					<br /><br /><i><?php echo translate('select_date_period'); ?></i>
				</div>
			</fieldset>
			<?php
			hidden_field($prefix.'_setting_single_date', ($using_single_date) ? 'true' : 'false');
		} else {
			$field->printField();
			if ($this->_edit_params['use_picker']) {
				// print the JS calendar popup date selecta
				require_once SQ_LIB_PATH.'/js_calendar/js_calendar.inc';

				$calendar = new JS_Calendar();
				$calendar->changeSetting('onDayClick', 'datetime_set_date');
				$calendar->paint(str_replace(':', '_', $prefix), '', TRUE);
			}
		}

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function process($prefix)
	{
		$setting_single_date = TRUE;
		if (isset($_REQUEST[$prefix.'_setting_single_date']) && $_REQUEST[$prefix.'_setting_single_date'] == 'false') {
			$setting_single_date = FALSE;
		}

		$using_single_date = is_iso8601($this->value);

		if ($setting_single_date) {
			$field_value = $this->value;
			if (!$using_single_date) {
				$field_value = '---------- --:--:--';
			}

			$field = new Datetime_Field($prefix, $field_value, $this->_edit_params);
			if ($field->processField()) {
				if ($this->value != $field_value && $this->setValue($field_value)) {
					$this->processed = TRUE;
				} else {
					$this->processed = FALSE;
				}
			} else {
				$this->processed = FALSE;
			}
		} else {
			$period_num = (isset($_REQUEST[$prefix.'period_num'])) ? (int) $_REQUEST[$prefix.'period_num'] : '1';
			$period_type = (isset($_REQUEST[$prefix.'_period_type'])) ? $_REQUEST[$prefix.'_period_type'] : 'd';
			$period_age = (isset($_REQUEST[$prefix.'_period_age'])) ? $_REQUEST[$prefix.'_period_age'] : '-';

			$period_options = $this->_getPeriodOptions();
			$age_options = $this->_getAgeOptions();

			if (!isset($period_options[$period_type])) {
				$period_type = 'd';
			}
			if (!isset($age_options[$period_age])) {
				$period_age = '-';
			}
			$value = $period_age.$period_num.$period_type;
			if ($this->value != $value && $this->setValue($value)) {
				$this->processed = TRUE;
			} else {
				$this->processed = FALSE;
			}
		}

	}//end process()


	/**
	* Get the current value as a unix timestamp
	*
	* @return int
	* @access public
	*/
	function getTimestamp()
	{
		$using_single_date = is_iso8601($this->value);

		if ($using_single_date) {
			$date_field_value = $this->value;
		} else {
			$matches = Array();
			$period_options = $this->_getPeriodOptions();
			$age_options = $this->_getAgeOptions();

			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			$date_field_value = date('Y-m-d h:i:s', strtotime($period_age.$period_num.' '.$period_options[$period_type]));
		}

		$field = new Datetime_Field('blah', $date_field_value, $this->_edit_params);
		$units = $field->strToUnitsArray($date_field_value);
		foreach (Array('y', 'm', 'd') as $u) {
			if ($units[$u] == -1) return -1;
		}

		// now we know that we can formulate a date
		$strtotime = $units['m'].'/'.$units['d'].'/'.$units['y'];

		$dotime = TRUE;
		foreach (Array('h', 'i', 's') as $u) {
			if ($units[$u] == -1) $dotime = FALSE;
		}
		if ($dotime) {
			$strtotime .= ' '.$units['h'].':'.$units['i'].':'.$units['s'];
		}

		return strtotime($strtotime);

	}//end getTimestamp()


	/**
	* Get the current value as an ISO8601 formatted date
	*
	* @return string
	* @access public
	*/
	function getISO8601()
	{
		$using_single_date = is_iso8601($this->value);

		if ($using_single_date) {
			return $this->value;
		} else {
			$matches = Array();
			$period_options = $this->_getPeriodOptions();
			$age_options = $this->_getAgeOptions();
			$time_zone_designater = date('O');
			$tzd_iso8601 = substr($time_zone_designater, 0, 3).':'.substr($time_zone_designater, -2);

			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			return date('Y-m-d\TH:i:s', strtotime($period_age.$period_num.' '.$period_options[$period_type])).$tzd_iso8601;
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

		if (is_iso8601($value)) {
			$ref = '';
			$field = new Datetime_Field('blah', $ref, $this->_edit_params);
			$value = $field->validateValue($value);
		} else {
			if (!empty($value) && !preg_match('|[\-\+][0-9]+['.implode('', array_keys($this->_getPeriodOptions())).']|', $value)) {
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
			$format_value = format_date(iso8601_ts($this->value), $format);
			if ($format_value) {
				return $format_value;
			}
		}
		return parent::getKeywordValue($format);

	}//end getKeywordValue()


}//end class

?>
