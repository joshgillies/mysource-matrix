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
* $Id: design_area_js_calendar_navigator.inc,v 1.1 2004/09/10 04:23:40 arailean Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_JS_Calendar_Navigator
*
* Purpose: Control a Calendar Asset
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator extends Design_Area
{

	/*
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	*
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printArea(&$design)
	{
		$php_code = '
		<!-- generated code -->
		<?php
			$calendar_navigator = &$GLOBALS["SQ_SYSTEM"]->am->getAsset('.$this->id.');
			$element_id 		= $calendar_navigator->attr("element_id");
			$calendar_assetid 	= $calendar_navigator->attr("target_assetid");

			$am =& $GLOBALS["SQ_SYSTEM"]->am;

			if (!empty($calendar_assetid)) {
				$asset =& $am->getAsset($calendar_assetid);
				$target_url = $asset->getURL();
			};

			$cal_path = sq_web_path("fudge")."/js_calendar";
			$cal_js_path = $cal_path."/cal_navigator.js";

			$custom_style_assetid = $calendar_navigator->attr("style_assetid");
			if (!empty($custom_style_assetid)) {
				$style_file =& $am->getAsset($custom_style_assetid);
				$style_url = $style_file->getURL();
			} else {
				$style_url = "$cal_path/cal_navigator.css";
			}

		?>
		<script src="<?php echo($cal_js_path);?>"></script>
		<link rel="stylesheet" type="text/css" href="<?php echo($style_url);?>" media="screen">


		<div id="<?php echo($element_id);?>"></div>

		<script type="text/javascript">

		<?php
		if (!empty($target_url)) {
		?>
			function onDayClick(day, mon, year)
			{
				document.location.href = "<?php echo($target_url);?>?SQ_CALENDAR_VIEW=day&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year);
			}

			function onWeekClick(day, mon, year)
			{
				document.location.href = "<?php echo($target_url);?>?SQ_CALENDAR_VIEW=week&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year);
			}

			function onMonthClick(day, mon, year)
			{
				document.location.href = "<?php echo($target_url);?>?SQ_CALENDAR_VIEW=month&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year);
			}

			function onYearClick(day, mon, year)
			{
				document.location.href = "<?php echo($target_url);?>?SQ_CALENDAR_VIEW=year&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year);
			}
		<?php
		}
		?>

			var cal = new Calendar("cal","<?php echo($element_id);?>");
			cal.onDayClick	= onDayClick;
			cal.onWeekClick	= onWeekClick;
			cal.onMonthClick= onMonthClick;
			cal.onYearClick	= onYearClick;
			cal.draw();
		</script>
		';

		echo $php_code;
		return true;

	}//end printArea()


}//end class

?>
