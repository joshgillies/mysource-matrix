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
* $Id: js_calendar.inc,v 1.3 2004/09/14 13:46:06 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


/**
* JS_Calendar
*
* Purpose
*     An object to init and paint the JS calendar
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
* @subpackage lib
*/
class JS_Calendar
{

	/**
	* An array of settings for the JS Calendar
	* @var array
	*/
	var $settings = Array(
							'cssURL'      => null,
							'imageURL'    => null,
							'fadeit'      => true,
							'scrollit'    => false,
							'onDayClick'  => null,
							'onWeekClick' => null,
							'onYearClick' => null,
							'height'      => 200,
							'width'       => 200,

						 );


	/**
	* Constructor
	*/
	function JS_Calendar()
	{

	}//end constructor


	/**
	* Change a setting for the calendar
	*
	* These settings are passed directly to the JS calendar object
	*
	* @access public
	* @return void
	*/
	function changeSetting($name, $value)
	{
		$this->settings[$name] = $value;

	}//end changeSetting()


	/**
	* Initiaise the JS calendar if it has not already been
	*
	* A global var is set to ensure this function only ever outputs content once. It can be
	* called as many times as you want but it will only output content the first time (unless you
	* get tricky with the global vars)
	*
	* @access public
	* @return boolean
	*/
	function init()
	{
		if (!$this->isInit()) {
			$css = array_get_index($this->settings,'cssURL',sq_web_path('fudge').'/js_calendar/js_calendar.css');
			?>
			<!-- Start init of JS Calendar -->
			<style type="text/css">
				@import url("<?php echo $css; ?>");
			</style>
			<script src="<?php echo sq_web_path('fudge'); ?>/js_calendar/js_calendar.js" language="Javascript" type="text/javascript"></script>
			<!--span id="sq_js_calendar_span"></span-->
			<!-- End init of JS Calendar -->
			<?php
			$GLOBALS['SQ_JS_CALENDAR_INIT'] = true;
		}

	}//end init()


	/**
	* Check if the JS Calendar has been initialised or not
	*
	* @access public
	* @return boolean
	*/
	function isInit()
	{
		return (isset($GLOBALS['SQ_JS_CALENDAR_INIT']) && $GLOBALS['SQ_JS_CALENDAR_INIT'] === true);

	}//end isInit()


	/**
	* Prints the JS calendar
	*
	* @param string		$prefix			a prefix to identify this calendar from others on the page
	* @param string		$container_name	the id of the container (span, div, td etc) to hold the calendar,
	*									leave empty to use the default container
	* @param boolean	$popup			display this calendar in a popup or not
	*
	* @access public
	* @return void
	*/
	function paint($prefix, $container_name='', $popup=false)
	{
		$this->init();

		$this->settings['popup'] = $popup;
		if (!isset($this->settings['imageURL']) || is_null($this->settings['imageURL'])) {
			$this->settings['imageURL'] = sq_web_path('fudge').'/js_calendar/calendar.gif';
		}

		if (empty($container_name)) {
			$container_name = 'sq_js_calendar_span';
			if (!$popup) {
				$container_name = $prefix.'_js_calendar_span';
			}
		}
		?>
		<span id="<?php echo $container_name; ?>"></span>
		<script language="Javascript" type="text/javascript">
			<?php echo $prefix; ?>_cal = new Calendar("<?php echo $prefix; ?>_cal", "<?php echo $container_name; ?>");
			<?php echo $prefix; ?>_cal.prefix = "<?php echo $prefix; ?>";
			<?php
			foreach ($this->settings as $name => $value) {
				if (is_null($value) || $name == 'cssURL') continue;

				if (is_bool($value)) {
					$value = ($value) ? 'true' : 'false';
				} else if (!in_array($name, Array('onDayClick', 'onMonthClick', 'onYearClick', 'onWeekClick'))) {
					$value = '"'.$value.'"';
				}

				echo $prefix.'_cal.'.$name.' = '.$value.";\n";
			}
			?>
			<?php echo $prefix; ?>_cal.draw();
		</script>
		<?php

	}//end paint()


}//end class

?>