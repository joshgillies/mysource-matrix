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
* $Id: calendar_event_edit_fns.inc,v 1.2 2004/07/23 07:09:45 tbarrett Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Page_Edit_Fns
*
* Purpose
*
*
* @author  Luke Wright <lwright@squiz.net>
* @version $Revision: 1.2 $ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Calendar_Event_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end Calendar_Event_Edit_Fns()


	/**
	 * Print the entry boxes for event input, which use
	 * javascript to prevent the user from choosing
	 * something invalid (obviously server-side validation
	 * is done too)
	 */

	function paintSingleStartEndChooser($asset) 
	{
		$this->paintStartEndChooser($asset, 'event', '');
	}

	function paintStartEndChooser($event, $name, $changeFunc='') 
	{
		?>
		<script language="JavaScript">
		<!--
			  function make2digits(num) {
				  if (num < 10)
					  return "0"+num;
				  else
					  return num;
			  }
			function processEndEnabledClick(elt, name) {
				if (elt.checked) {
					enableDateField('<?php echo $name; ?>End'); 
					if (isChecked('<?php echo $name; ?>StartTimeEnabled')) {
						enableTimeField('<?php echo $name; ?>End'); 
					}
				} else { 
					disableDateField('<?php echo $name; ?>End'); 
					disableTimeField('<?php echo $name; ?>End'); 
				}
			}

			function processStartTimeClick(elt, name) {
				if (elt.checked) {
					enableTimeField(name+'Start');
					if (isChecked(name+'EndDateEnabled')) {
						enableTimeField(name+'End'); 
					}
				} else {
					disableTimeField(name+'Start'); disableTimeField(name+'End');
				}
			}

			function processEndTimeClick(elt, name) {
				if (elt.checked) {
					enableTimeField(name+'End'); 
					enableDateField(name+'End'); 
					checkBox(name+'EndDateEnabled'); 
					enableTimeField(name+'Start'); 
				} else { 
					disableTimeField(name+'End'); 
					disableTimeField(name+'Start');  
				}
			}
			
			var dateComponents = Array('Year','Month','Day');
			
			function enableDateField(name) {
				for (i=0; i<dateComponents.length; i++) {
				  if ((elt = document.getElementById(name+dateComponents[i])) != null) {
					elt.disabled = 0;
				  }
				}
			  }

			 function disableDateField(name) {
				for (i=0; i<dateComponents.length; i++) {
				  if ((elt = document.getElementById(name+dateComponents[i])) != null) {
					elt.disabled = 1;
				  }
				}
			 }

			 function enableTimeField(name) {
				document.getElementById(name+'Hours').disabled=0;
				document.getElementById(name+'Minutes').disabled=0;
				document.getElementById(name+'IsPm').disabled=0;
				checkBox(name+'TimeEnabled');
			 }

			 function disableTimeField(name) {
				document.getElementById(name+'Hours').disabled=1;
				document.getElementById(name+'Minutes').disabled=1;
				document.getElementById(name+'IsPm').disabled=1;
				uncheckBox(name+'TimeEnabled');
			 }

			  function checkBox(id) {
				  if ((elt = document.getElementById(id)) != null)
					elt.checked=1;
				  else
					alert("Javascript error:  could not find box " + id + " to check");
			  }
			  function uncheckBox(id) {
				  if ((elt = document.getElementById(id)) != null)
					elt.checked=0;
				  else
					alert("Javascript error:  could not find box " + id + " to uncheck");
			  }
			  function isChecked(id) {
				  if ((elt = document.getElementById(id)) != null)
					return elt.checked;
				  else
					alert("Javascript error:  could not find box " + id + " so can't look at its status");
				  return 0;
			  }
			  function validateDay(elt) {
				  dayValue = parseInt(elt.value);
				  if ((dayValue == null) || (dayValue < 0) || (dayValue > 31)) {
					  alert("You entered an invalid day of the month");
					  elt.focus();
				  }
			  }
			  function validateMinutes(elt) {
				  minutesValue = parseInt(elt.value);
				  if ((minutesValue == null) || (minutesValue < 0) || (minutesValue > 59)) {
					  alert("You entered an invalid time");
					  elt.focus();
				  }
			  }
			  function validateHours(elt) {
				  hoursValue = parseInt(elt.value);
				  if ((hoursValue > 12) && (hoursValue < 24)) {
					  elt.value = hoursValue - 12;
					  if ((pmElt = document.getElementById(elt.getAttribute('id').substring(0, elt.getAttribute('id').length-5)+'IsPm')) != null)
						  pmElt.selectedIndex=1;
					  else
						  alert("Couldn't find " + substring(elt.getAttribute('id'), 0, elt.getAttribute('id').length-5)+'IsPm');
				  }
			  }

			  function validateFutureYear(elt) {
				  yearVal = parseInt(elt.value);
				  if (yearVal == null) {
					  alert("You entered an invalid year");
					  elt.value = '';
					  elt.focus();
				  } else {
					  if (yearVal < 100) {
						  yearVal = yearVal + 2000;
						  elt.value = yearVal + 2000;
					  }
				  }
			  }
			  function getDateFromField(name) {
				d = new Date(document.getElementById(name+'Year').value, document.getElementById(name+'Month').value, document.getElementById(name+'Day').value);
				hoursVal = parseInt(document.getElementById(name+'Hours').value);
				if ((document.getElementById(name+'IsPm').selectedIndex == 1) && (hoursVal < 12)) {
					hoursVal += 12;
				}
				if ((document.getElementById(name+'IsPm').selectedIndex == 0) && (hoursVal == 12)) {
					hoursVal = 0;
				}
			    d.setHours(hoursVal);
				d.setMinutes(document.getElementById(name+'Minutes').value);
				return d;
			  }

			  function setDateField(fieldName, dateVal) {
				document.getElementById(fieldName+'Year').value = dateVal.getFullYear();
				document.getElementById(fieldName+'Month').value = dateVal.getMonth();
				document.getElementById(fieldName+'Day').value = dateVal.getDate();
				if ((hourElt = document.getElementById(fieldName+'Hours')) != null) {
				  document.getElementById(fieldName+'IsPm').selectedIndex = ((dateVal.getHours() >= 12) ? 1 : 0);
				  if (dateVal.getHours() == 0)
					  hourElt.value = 12;
				  else
					  hourElt.value = make2digits(dateVal.getHours());
				  document.getElementById(fieldName+'Minutes').value = dateVal.getMinutes();
				}
			  }

			  function updateEndDate(name) {
				endD = getDateFromField(name+'End');
				startD = getDateFromField(name+'Start');
				//alert("Got end and start dates for updating end date");
				if (endD < startD)
				  setDateField(name+'End', startD);
			  }

			  function updateStartDate(name) {
				endD = getDateFromField(name+'End');
				startD = getDateFromField(name+'Start');
				//alert("Got end and start dates for updating start date");
				if (endD < startD)
				  setDateField(name+'Start', endD);
			  }
				
		//-->
		</script>
		<table border="0" cellpadding="0" cellspacing="0">
			<tr>
				<td class="sq-backend-data" style="text-align: right">Occurs on&nbsp;&nbsp;</td>
				<td class="sq-backend-data">
					<?php $this->printDateChooser($name.'Start', "updateEndDate('" . $name . "'); " . $changeFunc, ($event->getStartDate() ? iso8601_ts($event->attr('start_date')) : 0)); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($name.'Start', "updateEndDate('" . $name . "'); " .  $changeFunc, "processStartTimeClick(this, '$name'); ", ($event->getStartTime() ? iso8601_ts($event->attr('start_date')) : false)); ?>
				</td>
			</tr>
			<tr>
				<td class="sq-backend-data" style="text-align: right">
					<input type="checkbox" value="1" <?php if ($event->getEndDate()) echo ' checked="checked" '; ?> id="<?php echo $name; ?>EndDateEnabled" name="<?php echo $name; ?>EndDateEnabled" onclick="processEndEnabledClick(this, '<?php echo $name; ?>')" onkeypress="processEndEnabledClick(this, '<?php echo $name; ?>')" />runs until&nbsp;&nbsp;
				</td>
				<td class="sq-backend-data">
					<?php $this->printDateChooser($name.'End',"updateStartDate('" . $name . "'); " .  $changeFunc, ($event->getEndDate() ? iso8601_ts($event->attr('end_date')) : false)); ?>
				</td>
				<td class="sq-backend-data">
					&nbsp;&nbsp;<?php $this->printTimeChooser($name.'End', "updateStartDate('" . $name . "'); " .  $changeFunc, "processEndTimeClick(this, '$name'); ", ($event->getEndTime() ? iso8601_ts($event->attr('end_date')) : false)); ?>
				</td>
			</tr>
		</table>
		<?php
	}

	
	function processSingleStartEndChooser(&$asset, &$o, $prefix) 
	{
		$this->processStartEndChooser($asset, 'event');
	}

	function _make2digits($num) {
		if ($num < 10) $num = '0' . $num;
		return $num;
	}
	
	function processStartEndChooser(&$event, $name) 
	{
		foreach (Array('Start', 'End') as $component) {
			if (isset($_REQUEST[$name . $component . 'TimeEnabled'])) {
				// interpret AM/PM values and covert to 24hr time
				if ($_REQUEST[$name . $component . 'IsPm']) {
					$_REQUEST[$name . $component . 'Hours'] += 12;
				} else {
					if ($_REQUEST[$name . $component . 'Hours'] == 12) {
						$_REQUEST[$name . $component . 'Hours'] = 0;
					}
				}
				$_REQUEST[$name . $component . 'Hours'] = (int)$_REQUEST[$name . $component . 'Hours'];
				if (($_REQUEST[$name . $component . 'Hours'] < 0) || ($_REQUEST[$name . $component . 'Hours'] > 23)) {
					trigger_error('You entered an invalid hour', E_USER_WARNING);
					return false;
				}
				$_REQUEST[$name . $component . 'Minutes'] = (int)$_REQUEST[$name . $component . 'Minutes'];
				if (($_REQUEST[$name . $component . 'Minutes'] < 0) || ($_REQUEST[$name . $component . 'Minutes'] > 59)) {
					trigger_error('You entered an invalid minute', E_USER_WARNING);
					return false;
				}
			}
			if (isset($_REQUEST[$name . $component . 'Year'])) {
				$_REQUEST[$name . $component . 'Year'] = (int)$_REQUEST[$name . $component . 'Year'];
				if (($_REQUEST[$name . $component . 'Year'] < 1900) || ($_REQUEST[$name . $component . 'Year'] > 2100)) {
					trigger_error('You entered an invalid year value', E_USER_WARNING);
					return false;
				}
				$_REQUEST[$name . $component . 'Month'] = (int)$_REQUEST[$name . $component . 'Month'];
				if (($_REQUEST[$name . $component . 'Month'] < 1) || ($_REQUEST[$name . $component . 'Month'] > 12)) {
					trigger_error('You entered an invalid month value', E_USER_WARNING);
					return false;
				}
				$_REQUEST[$name . $component . 'Day'] = (int)$_REQUEST[$name . $component . 'Day'];
				if (($_REQUEST[$name . $component . 'Day'] < 1) || ($_REQUEST[$name . $component . 'Day'] > 31)) {
					trigger_error('You entered an invalid day value', E_USER_WARNING);
					return false;
				}
			}
		}

		$start_date_value = $_REQUEST[$name . 'StartYear'] .'-' 
							. $this->_make2digits($_REQUEST[$name . 'StartMonth']) .'-' 
							. $this->_make2digits($_REQUEST[$name . 'StartDay']) . ' ';
		if (isset($_REQUEST[$name . 'StartTimeEnabled'])) {
			$start_date_value .= $this->_make2digits($_REQUEST[$name . 'StartHours']) . ':'
									. $this->_make2digits($_REQUEST[$name . 'StartMinutes']) . ':--';
		} else {
			$start_date_value .= '--:--:--';
		}

		if (isset($_REQUEST[$name . 'EndTimeEnabled'])) {
			$end_date_value = $_REQUEST[$name . 'EndYear'] .'-' 
							. $this->_make2digits($_REQUEST[$name . 'EndMonth']) .'-' 
							. $this->_make2digits($_REQUEST[$name . 'EndDay']) . ' '
							. $this->_make2digits($_REQUEST[$name . 'EndHours']) . ':'
							. $this->_make2digits($_REQUEST[$name . 'EndMinutes']) . ':--';
		} else {
			if (isset($_REQUEST[$name . 'EndDateEnabled'])) {
				$end_date_value = $_REQUEST[$name . 'EndYear'] .'-' 
							. $this->_make2digits($_REQUEST[$name . 'EndMonth']) .'-' 
							. $this->_make2digits($_REQUEST[$name . 'EndDay']) . ' '
							. '--:--:--';
			} else {
				$end_date_value = '---------- --:--:--';
			}
		}
		$res = true;
		$res &= $event->setAttrValue('start_date', $start_date_value);
		$res &= $event->setAttrValue('end_date', $end_date_value);
		return $res;
	}

	
	function printDateChooser($name, $changeFunc, $timestamp)
	{
		$disabled_text = ($timestamp === false) ? ' disabled="disabled" ' : '';
		if (!$timestamp) $timestamp = time();
		?>
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $name; ?>Day" id="<?php echo $name; ?>Day" size="2" value="<?php echo date('d', $timestamp); ?>" <?php echo $disabled_text; ?> onchange="validateDay(this); <?php echo $changeFunc; ?>" onfocus="this.select()" />
		
		<select class="sq-form-field" name="<?php echo $name; ?>Month" id="<?php echo $name; ?>Month" <?php echo $disabled_text; ?> onchange="<?php echo $changeFunc; ?>">
		<?php
		$currentMonth = date('n', $timestamp);
		foreach(range(1, 12) as $monthNumber) {
			echo '		<option ' . (($monthNumber == $currentMonth) ? ' selected="selected" ' : '') . 'value="' . $monthNumber . '">' . DateTime_Field::shortMonth($monthNumber) . "</option>\n";
		}
		?>
		</select>
		
		<input class="sq-form-field" type="text" name="<?php echo $name; ?>Year" id="<?php echo $name; ?>Year" size="2" value="<?php echo date('Y', $timestamp); ?>" <?php echo $disabled_text; ?> onchange="validateFutureYear(this); <?php echo $changeFunc; ?>" onfocus="this.select()" />
		<?php
	}

	function printTimeChooser($name, $changeFunc, $clickFunc, $timestamp)
	{
		$disabled_text = ($timestamp === false) ? ' disabled="disabled" ' : '';
		$checked_text = ($timestamp === false) ? '' : ' checked="checked" ';
		if (!$timestamp) $timestamp = time();

		?>
		<input class="sq-form-field" type="checkbox" value="1" <?php echo $checked_text; ?> name="<?php echo $name; ?>TimeEnabled" id="<?php echo $name; ?>TimeEnabled" onclick="<?php echo $clickFunc; ?>" onkeypress="<?php echo $clickFunc; ?>" />at
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $name; ?>Hours" id="<?php echo $name; ?>Hours" size="2" maxlength="2" value="<?php echo date('g', $timestamp); ?>" onchange="validateHours(this); <?php echo $changeFunc; ?>" <?php echo $disabled_text; ?> onfocus="this.select()" /> :
		
		<input class="sq-form-field" style="width: 5ex" type="text" name="<?php echo $name; ?>Minutes" id="<?php echo $name; ?>Minutes" size="2" maxlength="2" value="<?php echo date('i', $timestamp); ?>" onchange="validateMinutes(this); <?php echo $changeFunc; ?>" <?php echo $disabled_text; ?> onfocus="this.select()" />
		
		<select class="sq-form-field" name="<?php echo $name; ?>IsPm" id="<?php echo $name; ?>AmPm" onchange="<?php echo $changeFunc; ?>" <?php echo $disabled_text; ?> >
			<option value="0" <?php echo (date('a', $timestamp)=='am') ? ' selected="selected" ' : ''; ?>>am</option>
			<option value="1" <?php echo (date('a', $timestamp)=='pm') ? ' selected="selected" ' : ''; ?>>pm</option>
		</select>
		<?php
	}
		


}//end class

?>