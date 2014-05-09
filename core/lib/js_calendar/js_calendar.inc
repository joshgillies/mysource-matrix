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
* $Id: js_calendar.inc,v 1.11 2012/08/30 01:09:21 ewang Exp $
*
*/


/**
* JS_Calendar
*
* Purpose
*     An object to init and paint the JS calendar
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.11 $
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
						'cssURL'			=> null,
						'imageURL'			=> null,
						'fadeit'			=> true,
						'scrollit'			=> false,
						'onDayClick'		=> null,
						'onWeekClick'		=> null,
						'onYearClick'		=> null,
						'height'			=> 200,
						'width'				=> 200,
						'day_name_length'	=> 2,
						'week_start'		=> 1,
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
		// prevent xss attack
		$value = preg_replace('/[^a-zA-Z0-9\-\%_:\.\/]/', '', $value);
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
			<script src="<?php echo sq_web_path('fudge'); ?>/js_calendar/js_calendar.js" type="text/javascript"></script>
			<script type="text/javascript">
				addStyle('@import url("<?php echo $css; ?>");');
			</script>
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
	* @param string		$prefix				a prefix to identify this calendar from others on the page
	* @param string		$container_name		the id of the container (span, div, td etc) to hold the calendar,
	*										leave empty to use the default container
	* @param boolean	$popup				display this calendar in a popup or not
	* @param boolean	$container_exists	if true, the container wont be created,
	*										it must already exist in the html
	*										this parameter is only used if $container_name is specified
	*
	* @access public
	* @return void
	*/
	function paint($prefix, $container_name='', $popup=false, $container_exists=false)
	{
		$this->init();

		$this->settings['popup'] = $popup;
		if (!isset($this->settings['imageURL']) || is_null($this->settings['imageURL'])) {
			$this->settings['imageURL'] = sq_web_path('fudge').'/js_calendar/calendar.gif';
		}

		if (empty($container_name)) {
			$container_name = $prefix.'_js_calendar_span';
		}

		$name_add_on = '';
		if ($popup) {
			if (array_get_index($GLOBALS, 'SQ_JS_CALENDAR_POPUP', false) === false) {
				$GLOBALS['SQ_JS_CALENDAR_POPUP'] = $container_name;
				?><span id="<?php echo $container_name; ?>"></span><?php
			} else {
				$container_name = $GLOBALS['SQ_JS_CALENDAR_POPUP'];
			}
		} else {
			if (array_get_index($GLOBALS, $container_name, false) === false) {
				$GLOBALS[$container_name] = 1;
			} else {
				$GLOBALS[$container_name]++;
			}
			$name_add_on = ($GLOBALS[$container_name] == 1)?'':'_'.$GLOBALS[$container_name];
			?><span id="<?php echo $container_name.$name_add_on; ?>"></span><?php
		}
		if (!isset($this->settings['currentDate'])) {
			$this->settings['currentDate'] = date('Y-m-d');
		}

		list($year, $month, $day) = explode('-', $this->settings['currentDate']);
		?>
		<script type="text/javascript">
		//<![CDATA[
			<?php echo $prefix; ?>_cal = new Calendar("<?php echo $prefix; ?>_cal", "<?php echo $container_name.$name_add_on; ?>", "<?php echo $this->settings['width'];?>","<?php echo $this->settings['height'];?>","<?php echo $year;?>","<?php echo $month;?>","<?php echo $day;?>");
			<?php echo $prefix; ?>_cal.prefix = "<?php echo $prefix; ?>";
			<?php
			foreach ($this->settings as $name => $value) {
				if (is_null($value) || $name == 'cssURL'){
					continue;
				}

				if (is_bool($value)) {
					$value = ($value) ? 'true' : 'false';
				} else if (!in_array($name, Array('onDayClick', 'onMonthClick', 'onYearClick', 'onWeekClick'))) {
					$value = '"'.$value.'"';
				}

				echo $prefix.'_cal.'.$name.' = '.$value.";\n";
			}
			?>
			<?php echo $prefix; ?>_cal.draw();
		//]]>
		</script>
		<?php

	}//end paint()


}//end class

?>