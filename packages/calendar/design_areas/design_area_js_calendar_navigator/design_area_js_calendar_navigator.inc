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
* $Id: design_area_js_calendar_navigator.inc,v 1.3 2004/10/20 06:25:44 tbarrett Exp $
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
* @version $Version$ - 1.1
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
		<!-- generated calendar code -->
		<?php
			$am =& $GLOBALS["SQ_SYSTEM"]->am;

			$calendar_navigator = &$am->getAsset('.$this->id.');
			$element_id 		= $calendar_navigator->attr("element_id");
			$calendar_assetid 	= $calendar_navigator->attr("target_assetid");

			if (!empty($calendar_assetid)) {
				$asset = &$am->getAsset($calendar_assetid);
				$target_url = $asset->getURL();

				// generate javascript even handlers
				?>
				<script language="Javascript" type="text/javascript">
					function frameRedirect(url) 
					{
					    basetags = document.getElementsByTagName(\'base\');
						if (basetags.length != 0) {
							switch (basetags[0].target) {
								case \'_parent\':
									window.parent.location = url;
								  break;
								case \'_top\':
									window.top.location = url;
									break;
								case \'_self\':
									document.location.href = url;
									break;
								case \'_blank\':
									window.open(url, \'new\', \'\');
									break;
								default:
									window.top.frames[basetags[0].target].location = url;
							}
						} else {
							document.location.href = url;
						}
					}								
					function onDayClick(day, mon, year)
					{
						frameRedirect("<?php echo($target_url);?>?SQ_CALENDAR_VIEW=day&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year));
					}

					function onWeekClick(day, mon, year)
					{
						frameRedirect("<?php echo($target_url);?>?SQ_CALENDAR_VIEW=week&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year));
					}

					function onMonthClick(day, mon, year)
					{
						frameRedirect("<?php echo($target_url);?>?SQ_CALENDAR_VIEW=month&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year));
					}

					function onYearClick(day, mon, year)
					{
						frameRedirect("<?php echo($target_url);?>?SQ_CALENDAR_VIEW=year&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year));
					}
				</script>
				<?php


			};

			require_once SQ_LIB_PATH."/js_calendar/js_calendar.inc";

			$calendar = new JS_Calendar();
			$calendar->changeSetting("onDayClick", 	"onDayClick");
			$calendar->changeSetting("onWeekClick", "onWeekClick");
			$calendar->changeSetting("onMonthClick","onMonthClick");
			$calendar->changeSetting("onYearClick",	"onYearClick");
			$calendar->changeSetting("height",		"100%");
			$calendar->changeSetting("width",		"100%");

			$custom_style_assetid = $calendar_navigator->attr("style_assetid");
			if (!empty($custom_style_assetid)) {
				$style_file =& $am->getAsset($custom_style_assetid);
				$calendar->changeSetting("cssURL",  $style_file->getURL());
			}

			$calendar->paint("js_calendar",$element_id);

		?>
		<!-- end generated calendar code -->
		';

		echo $php_code;
		return true;

	}//end printArea()


}//end class

?>
