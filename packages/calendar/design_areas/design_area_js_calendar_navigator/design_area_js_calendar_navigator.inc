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
* $Id: design_area_js_calendar_navigator.inc,v 1.19 2012/08/30 00:57:28 ewang Exp $
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
* @version $Revision: 1.19 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Design_Area_JS_Calendar_Navigator extends Design_Area
{


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	* In the form
	*
	*   Array('[link_type]' => Array('[type_code]' => Array('card' => [cardinality], 'exclusive' => [exclusive])));
	* Where:
	*   link_type   = SQ_LINK_TYPE_1|SQ_LINK_TYPE_2|SQ_LINK_TYPE_3|SQ_LINK_NOTICE
	*   cardinality = [max number of links]|'M'
	*   exclusive   = TRUE|FALSE
	*
	* @return array
	* @access private
	*/
	function _getAllowedLinks()
	{
		$res = parent::_getAllowedLinks();
		$res[SQ_LINK_NOTICE]['page_calendar'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $res;

	}//end _getAllowedLinks()


	/**
	* Outputs either the straight HTML or PHP code that is needed for this design area to be represented
	* @param object Design	$design the design for which we are currently printing
	*
	* @return boolean
	* @access public
	*/
	function printPaintingCode()
	{
		$page_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_NOTICE, 'page_calendar', FALSE);
		$page_ids = Array();
		foreach ($page_links as $link) {
			$page_ids[substr($link['value'], 0, -5)] = $link['minorid'];
		}

		$open_php = '<'.'?'.'php';
		$close_php = '?'.'>';
		?>

		<?php echo $open_php; ?>
			$<?php echo $this->attr('id_name'); ?>_target_pages = <?php var_export($page_ids); ?>;
			$<?php echo $this->attr('id_name'); ?>_target_urls = $GLOBALS['SQ_SYSTEM']->am->getAssetURL(array_values($<?php echo $this->attr('id_name'); ?>_target_pages));
		<?php echo $close_php; ?>

		<script type="text/javascript">
		//<![CDATA[
			<?php
			foreach (Array('day', 'week', 'month', 'year') as $unit) {
				?>

				function on<?php echo ucfirst($unit); ?>Click(day, mon, year)
				{
					<?php echo $open_php; ?>
					if (!empty($<?php echo $this->attr('id_name'); ?>_target_pages['<?php echo $unit; ?>'])) {
						$cal_page_id = $<?php echo $this->attr('id_name'); ?>_target_pages['<?php echo $unit; ?>'];
						if (!empty($<?php echo $this->attr('id_name'); ?>_target_urls[$cal_page_id])) {
							<?php echo $close_php; ?>
							frameRedirect("<?php echo $open_php; ?> echo $<?php echo $this->attr('id_name'); ?>_target_urls[$cal_page_id]; <?php echo $close_php; ?>?SQ_CALENDAR_VIEW=<?php echo $unit; ?>&SQ_CALENDAR_DATE=" + DateConvert2MySQL(day, mon, year));
							<?php echo $open_php; ?>
						}
					}
					<?php echo $close_php; ?>
				}

				<?php
			}
			?>
		//]]>
		</script>

		<?php echo $open_php; ?>
		if (!empty($<?php echo $this->attr('id_name'); ?>_target_urls)) {
			require_once SQ_LIB_PATH."/js_calendar/js_calendar.inc";
			$calendar = new JS_Calendar();
			$calendar->changeSetting("onDayClick", 	"onDayClick");
			$calendar->changeSetting("onWeekClick", "onWeekClick");
			$calendar->changeSetting("onMonthClick","onMonthClick");
			$calendar->changeSetting("onYearClick",	"onYearClick");
			$calendar->changeSetting("height",		"100%");
			$calendar->changeSetting("width",		"100%");
			if (isset($_GET["SQ_CALENDAR_DATE"])) {
				$date = preg_replace('/[^0-9\-]/', '', $_GET["SQ_CALENDAR_DATE"]);
				$calendar->changeSetting("currentDate",	$date);
			}

			$custom_style_assetid = '<?php echo $this->attr('style_assetid'); ?>';
			if (!empty($custom_style_assetid)) {
				$style_file = $GLOBALS['SQ_SYSTEM']->am->getAsset($custom_style_assetid);
				$calendar->changeSetting("cssURL",  $style_file->getURL());
			}

			$calendar->paint("js_calendar", '<?php echo str_replace("'", "\\'", $this->attr('element_id')); ?>');
		}
		<?php echo $close_php; ?>

		<?php
		return TRUE;

	}//end printPaintingCode()


}//end class

?>
