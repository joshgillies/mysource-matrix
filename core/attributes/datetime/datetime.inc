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
* $Id: datetime.inc,v 1.21 2004/09/14 00:26:26 gsherwood Exp $
* $Name: not supported by cvs2svn $
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
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage attributes
*/
class Asset_Attribute_DateTime extends Asset_Attribute
{

	/**
	* Valid period type options for the datetime field
	* @var array
	*/
	var $_period_options = Array( 'i' => 'minutes',
								  'h' => 'hours',
								  'd' => 'days',
								  'w' => 'weeks',
								  'm' => 'months',
								  'y' => 'years',
								);
								
	/**
	* Valid period age options for the datetime field
	* @var array
	*/
	var $_age_options = Array( '-' => 'in the past',
							   '+' => 'in the future',
							);


	/**
	* Constructor
	*
	* @param int	$attributeid	the attribute id to be loaded
	* @param mixed	$value			the current value for the element
	*
	*/
	function Asset_Attribute_DateTime($attributeid=0, $value=null) 
	{
		$this->Asset_Attribute($attributeid, $value);

		// default values
		$this->_edit_params['allow_null']   = true;
		$this->_edit_params['max']          = '2030-12-31 23:59:59';
		$this->_edit_params['min']          = '1970-01-01 00:00:00';
		$this->_edit_params['allow_circa']  = '0';
		$this->_edit_params['print_format'] = 'l, F jS, Y H:i:s';
		$this->_edit_params['allow_period'] = false;
		$this->_edit_params['use_picker']   = false;

	}//end constructor


	/**
	* Takes an XML_Tree_Node from an edit interface XML file and creates
	* the params array from it
	*
	* @param object XML_Tree_Node	$node
	*
	* @return boolean
	* @access public
	*/
	function setEditParams(&$node)
	{
		if (!parent::setEditParams($node)) return false;

		if (!empty($node->attributes['min'])) $this->_edit_params['min'] = $node->attributes['min'];
		if (!empty($node->attributes['max'])) $this->_edit_params['max'] = $node->attributes['max'];
		if (!empty($node->attributes['allow_circa'])) $this->_edit_params['allow_circa'] = $node->attributes['allow_circa'];
		if (!empty($node->attributes['print_format'])) $this->_edit_params['print_format'] = $node->attributes['print_format'];
		
		// what to print and how to show it (textbox or select box)
		// need to do some more intense looking around for these options :)
		$this->_edit_params['show']  = Array();
		$this->_edit_params['style'] = Array();
		foreach ($node->children as $c => $child) {
			if ($child->name == 'show') {
				foreach ($child->children as $i => $format) {
					$this->_edit_params['show'][] = $format->name;
					$this->_edit_params['style'][$format->name] = $format->attributes['style'];
					if (isset($format->attributes['allow_null']) && $format->attributes['allow_null']) {
						$this->_edit_params['null'][] = $format->name;
					}
				}
				break;
			}
		}

		if (isset($node->attributes['allow_period']) && $node->attributes['allow_period']) {
			$this->_edit_params['allow_period'] = true;
		}

		if (isset($node->attributes['use_picker']) && $node->attributes['use_picker']) {
			$this->_edit_params['use_picker'] = true;
		}

		// if nothing was found, define some defaults of our own
		if (empty($this->_edit_params['show'])) {
			$this->_edit_params['show'] = Array('y','m','d','h','i','s');
			foreach($this->_edit_params['show'] as $u) {
				$this->_edit_params['style'][$u] = 's';
			}
		}
		
		return true;

	}//end setEditParams()


	/**
	* Prints the interface for filling in a value
	*
	* @param string		$prefix		prefix for the form element
	* @param boolean	$read_only	are we just printing the value
	*
	* @access public
	*/
	function paint($prefix, $read_only=false)
	{
		$using_single_date = is_iso8601($this->value);
		
		$period_num = '1';
		$period_type = 'd';
		$period_age = '-';
		
		if ($using_single_date) {
			$date_field_value = $this->value;
			
			if ($date_field_value != '---------- --:--:--') {

				// get the current value as a timestamp
				// NOTE: we replace -- with ZERO because strtotime needs a full iso8601 date
				// to get the timestamp - not just part of one
				$timestamp = strtotime(str_replace('--', '0', $this->value));
	
				if ($timestamp < time()) {
					$format = easy_time_total(time() - $timestamp, false, true);
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
			
		} else {
			$matches = Array();
			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($this->_period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			$date_field_value = date('Y-m-d h:i:s', strtotime($period_age.$period_num.' '.$this->_period_options[$period_type]));
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
			$start_text = 'Select Single Period';
		} else {
			if ($read_only) {
				echo $period_num.' '.$this->_period_options[$period_type].' '.$this->_age_options[$period_age];
				return;
			}
			$start_text = 'Select Date Period';
		}

		if ($this->_edit_params['allow_period']) {
			?>
			<fieldset>
				<legend>
					<span id="<?php echo $prefix; ?>_selecta"><?php echo $start_text; ?></span>
					[
					<a class="sq-backend-legend" href="#"
					style="text-decoration: none;"
					onclick="Javascript: sq_toggle_double_div('<?php echo $prefix; ?>_date_field', '<?php echo $prefix; ?>_period_field', '<?php echo $prefix; ?>_selecta', 'Select Single Date', 'Select Date Period'); set_hidden_field('<?php echo $prefix; ?>_setting_single_date', document.getElementById('<?php echo $prefix; ?>_date_field').style.display == 'block'); return false;">change</a>
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
							$calendar->paint($prefix, $prefix.'_cal_picker', true);
						}
					?>
					<span id="<?php echo $prefix; ?>_cal_picker"></span>
					<br /><br /><i>Select a single date using the boxes above</i>
				</div>
				<div id="<?php echo $prefix; ?>_period_field" style="display: <?php echo ($using_single_date) ? 'none' : 'block'; ?>;">
					<?php
					text_box($prefix.'period_num', $period_num, 3);
					echo '&nbsp;';
					combo_box($prefix.'_period_type', $this->_period_options, false, $period_type);
					echo '&nbsp;';
					combo_box($prefix.'_period_age', $this->_age_options, false, $period_age);
					?>
					<br /><br /><i>Enter a period using the boxes above. eg. 3 days</i>
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
				$calendar->paint($prefix, '', true);
			}
		}

	}//end paint()


	/**
	* Prints the interface for filling in a value
	*
	* @param string	prefix	prefix for the form element
	*
	* @access public
	*/
	function process($prefix)
	{
		$setting_single_date = true;
		if (isset($_REQUEST[$prefix.'_setting_single_date']) && $_REQUEST[$prefix.'_setting_single_date'] == 'false') {
			$setting_single_date = false;
		}

		$using_single_date = is_iso8601($this->value);
		
		if ($setting_single_date) {
			$field_value = $this->value;
			if (!$using_single_date) $field_value = '---------- --:--:--';

			$field = new Datetime_Field($prefix, $field_value, $this->_edit_params);
			if ($field->processField()) {
				if ($this->value != $field_value && $this->setValue($field_value)) {
					$this->processed = true;
				} else {
					$this->processed = false;
				}
			} else {
				$this->processed = false;
			}
		} else {
			$period_num = (isset($_REQUEST[$prefix.'period_num'])) ? (int) $_REQUEST[$prefix.'period_num'] : '1';
			$period_type = (isset($_REQUEST[$prefix.'_period_type'])) ? $_REQUEST[$prefix.'_period_type'] : 'd';
			$period_age = (isset($_REQUEST[$prefix.'_period_age'])) ? $_REQUEST[$prefix.'_period_age'] : '-';

			if (!isset($this->_period_options[$period_type])) $period_type = 'd';
			if (!isset($this->_age_options[$period_age])) $period_age = '-';

			$value = $period_age.$period_num.$period_type;
			if ($this->value != $value && $this->setValue($value)) {
				$this->processed = true;
			} else {
				$this->processed = false;
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
			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($this->_period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			$date_field_value = date('Y-m-d h:i:s', strtotime($period_age.$period_num.' '.$this->_period_options[$period_type]));
		}

		$field = new Datetime_Field('blah', $date_field_value, $this->_edit_params);
		$units = $field->strToUnitsArray($date_field_value);
		foreach(Array('y', 'm', 'd') as $u) if ($units[$u] == -1) return -1;

		// now we know that we can formulate a date
		$strtotime = $units['m'].'/'.$units['d'].'/'.$units['y'];

		$dotime = true;
		foreach(Array('h', 'i', 's') as $u) if ($units[$u] == -1) $dotime = false;
		if ($dotime) $strtotime .= ' '.$units['h'].':'.$units['i'].':'.$units['s'];

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
			preg_match('|([\-\+])([0-9]+)(['.implode('', array_keys($this->_period_options)).'])|', $this->value, $matches);
			if (isset($matches[1])) $period_age  = $matches[1];
			if (isset($matches[2])) $period_num  = $matches[2];
			if (isset($matches[3])) $period_type = $matches[3];
			return date('Y-m-d h:i:s', strtotime($period_age.$period_num.' '.$this->_period_options[$period_type]));
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
			return true;
		}

		$orig_value = $value;

		if (is_iso8601($value)) {
			$ref = '';
			$field = new Datetime_Field('blah', $ref, $this->_edit_params);
			$value = $field->validateValue($value);
		} else {
			if (!preg_match('|[\-\+][0-9]+['.implode('', array_keys($this->_period_options)).']|', $value)) {
				$value = '-1d';
			}
		}
		return true;

	}//end validateValue()


	/**
	* Retrieve a list of indexable values from this attribute
	*
	* The return value not only contains the indexable values, but also the number of
	* times they occur in this attributes value
	*
	* @return array(string => int)
	* @access public
	*/
	function getKeyData()
	{
		return Array($this->value => 1);

	}//end getKeyData()


}//end class

?>
