<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: design_area_js_calendar_navigator.inc,v 1.11.2.1 2006/09/15 01:19:16 tbarrett Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_JS_Calendar_Navigator
*
* Purpose: Control a Calendar Asset
*
*
* @author  Andrei Railean <arailean@squiz.net>
* @version $Revision: 1.11.2.1 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator extends Design_Area
{


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printPaintingCode()
	{
		$php_code = '
		<!-- start calendar navigator -->
		<?php
		$am =& $GLOBALS["SQ_SYSTEM"]->am;
		$calendar_navigator = &$am->getAsset('.$this->id.');
		$element_id 		= $calendar_navigator->attr("element_id");
		$calendar_assetid 	= $calendar_navigator->attr("target_assetid");

		if (!empty($calendar_assetid)) {
			$asset = &$am->getAsset($calendar_assetid);
			$target_url = $asset->getURL();

			// generate javascript event handlers
			?>
			<script type="text/javascript">
			//<![CDATA[
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
			//]]>
			</script>

			<?php
			require_once SQ_LIB_PATH."/js_calendar/js_calendar.inc";
			$calendar = new JS_Calendar();
			$calendar->changeSetting("onDayClick", 	"onDayClick");
			$calendar->changeSetting("onWeekClick", "onWeekClick");
			$calendar->changeSetting("onMonthClick","onMonthClick");
			$calendar->changeSetting("onYearClick",	"onYearClick");
			$calendar->changeSetting("height",		"100%");
			$calendar->changeSetting("width",		"100%");
			if (isset($_GET["SQ_CALENDAR_DATE"])) {
				$calendar->changeSetting("currentDate",	$_GET["SQ_CALENDAR_DATE"]);
			}

			$custom_style_assetid = $calendar_navigator->attr("style_assetid");
			if (!empty($custom_style_assetid)) {
				$style_file =& $am->getAsset($custom_style_assetid);
				$calendar->changeSetting("cssURL",  $style_file->getURL());
			}

			$calendar->paint("js_calendar",$element_id);
		}
		?>
		<!-- end calendar navigator -->
		';

		echo $php_code;
		return TRUE;

	}//end printPaintingCode()


}//end class

?>
