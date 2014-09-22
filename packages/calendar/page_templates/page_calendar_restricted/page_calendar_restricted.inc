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
* $Id: page_calendar_restricted.inc,v 1.12 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';
require_once SQ_FUDGE_PATH.'/general/text.inc';
require_once SQ_PACKAGES_PATH.'/calendar/lib/calendar_common.inc';
require_once SQ_PACKAGES_PATH.'/calendar/page_templates/page_calendar_rolling/page_calendar_rolling.inc';

/**
* Page_Calendar_Restricted
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.12 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Page_Calendar_Restricted extends Page_Calendar_Rolling
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);
	}//end constructor


	/**
	* Add valid keywords for this asset to an array of keywords when asked
	*
	* @param object	&$broadcaster	the asset that triggered the event
	* @param array	$vars			the vars that get submitted by the broadcaster
	*								we add keywords to the $vars['keywords'] array
	*
	* @return boolean
	* @access private
	*/
	function onRequestKeywords(&$broadcaster, $vars=Array())
	{
		if (!isset($vars['keywords'])) {
			$vars['keywords'] = Array();
		}

		// get the bodycopy parent of the broadcaster
		$broadcaster_parents = $GLOBALS['SQ_SYSTEM']->am->getParents($broadcaster->id);
		$format_bodycopy_link = reset($GLOBALS['SQ_SYSTEM']->am->getLinks($this->id, SQ_LINK_TYPE_2, 'bodycopy', TRUE, 'major', '', 'TRUE'));
		if (!empty($format_bodycopy_link['minorid']) && isset($broadcaster_parents[$format_bodycopy_link['minorid']])) {
			// this is the page_contents bodycopy
			$vars['keywords']['next_link'] = 'Link to the next page of events';
			$vars['keywords']['prev_link'] = 'Link to the previous page of events';
		}

		return parent::onRequestKeywords($broadcaster, $vars);

	}//end onRequestKeywords()


	/**
	* Get the keyword replacements to use in the page contents bodycopy
	*
	* @return array
	* @access protected
	*/
	function _getPageContentsReplacements()
	{
		$res = parent::_getPageContentsReplacements();
		list($current_start_date, $current_last_date) = $this->_getDisplayDates();
		$first_valid_date = substr($this->attr('start_date'), 0, 10);
		$last_valid_date = substr($this->attr('end_date'), 0, 10);
		$last_normal_start = date('Y-m-d', strtotime(substr($this->attr('end_date'), 0, 10).' -'.($this->attr('days_per_page')-1).' days'));

		$res['next_link'] = '';
		if ($current_last_date < $last_valid_date) {
			$next_date = date('Y-m-d', strtotime($current_start_date.' +'.($this->attr('days_per_page')).' days'));
			$num_days_next = $this->attr('days_per_page');
			if ($next_date > $last_normal_start) {
				$num_days_next = days_between_isos($last_valid_date, $next_date) + 1;
			}
			$url = htmlspecialchars(replace_query_string_vars(Array('show_date' => $next_date)));
			$text = str_replace('%n%', $num_days_next, $this->attr('next_text'));
			$res['next_link'] = '<a href="'.$url.'">'.$text.'</a>';
		}

		$res['prev_link'] = '';
		if ($current_start_date > $first_valid_date) {
			$last_date = date('Y-m-d', strtotime($current_start_date.' -'.$this->attr('days_per_page').' days'));
			$num_days_next = $this->attr('days_per_page');
			if ($last_date < $first_valid_date) {
				$last_date = $first_valid_date;
				$num_days_next = days_between_isos($current_start_date, $last_date) + 1;
			}
			$url = htmlspecialchars(replace_query_string_vars(Array('show_date' => $last_date)));
			$text = str_replace('%n%', $num_days_next, $this->attr('prev_text'));
			$res['prev_link'] = '<a href="'.$url.'">'.$text.'</a>';
		}

		return $res;

	}//end _getPageContentsReplacements()


	/**
	* Get the events that should be displayed in this calendar page
	* Returns: (date => array (assetid => array ( details )))
	*
	* @return array
	* @access public
	*/
	function _getEvents()
	{
		list($first_date, $last_date) = $this->_getDisplayDates();
		return parent::_getEvents($first_date, $last_date);

	}//end _getEvents()


	/**
	* Get the first and last dates to display on this page in its current state
	* Returns: (first_date, last_date)
	*
	* @return array
	* @access private
	*/
	function _getDisplayDates()
	{
		$start_date = substr($this->attr('start_date'), 0, 10);
		$end_date = substr($this->attr('end_date'), 0, 10);
		$length = $this->attr('days_per_page');
		$display_date = array_get_index($_REQUEST, 'show_date', date('Y-m-d'));
		if ($display_date < $start_date) {
			$display_date = $start_date;
		}
		$last_date = date('Y-m-d', strtotime($display_date.' +'.($length-1).' days'));
		if ($last_date > $end_date) $last_date = $end_date;
		return Array($display_date, $last_date);

	}//end _getDisplayDates()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$end_date = substr($this->attr('end_date'), 0, 10);
		if ($end_date == '----------') {
			trigger_localised_error('CAL0063', sprintf(translate('Lastest date to show for Restricted Period Calendar Page #%s has not been set.'), $this->id), E_USER_WARNING)
			return;
		}
		parent::printBody();

	}//end printBody()


}//end class
?>
