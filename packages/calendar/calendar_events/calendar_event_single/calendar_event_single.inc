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
* $Id: calendar_event_single.inc,v 1.24 2012/08/30 00:57:28 ewang Exp $
*
*/

require_once  dirname(__FILE__).'/../../calendar_event/calendar_event.inc';

/**
* Calendar_Event_Single
*
* Purpose
*
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Tom Barrett <tbarrett@squiz.net>
* @version $Revision: 1.24 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Single extends Calendar_Event
{
	var $parent_links = Array();


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Calendar_Event_Single($assetid=0)
	{
		$this->Calendar_Event($assetid);

	}//end constructor


	/**
	* Load the asset represented by the passed asset ID
	*
	* @param int	$assetid	the asset ID to be loaded
	*
	* @return void
	* @access public
	*/
	function load($assetid)
	{
		parent::load($assetid);

		$this->parent_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_SC_LINK_ALL, 'calendar_event_multi_date', FALSE, 'minor');

	}//end load()


	/**
	* Perform any pre-validation required during the creation of this asset
	*
	* Single events ensure their name reflects their association with cancellations
	*
	* @param array	&$link	information used to create the initial link
	*
	* @return boolean
	* @access private
	*/
	function _preCreateCheck(&$link)
	{
		if ($link['asset'] instanceof Calendar_Event_Cancellation) {
			$replacee_link = $GLOBALS['SQ_SYSTEM']->am->getLink($link['asset']->id, NULL, '', TRUE, NULL, 'minor');
			$replacee = $GLOBALS['SQ_SYSTEM']->am->getAsset($replacee_link['majorid']);
			if (substr($this->attr('start_date'), 0, 10) == substr($link['asset']->attr('start_date'), 0, 10)) {
				$this->setAttrValue('name', 'Modified: '.$replacee->name);
			} else {
				$this->setAttrValue('name', 'Rescheduled from '.date('jS M Y', iso8601_ts($link['asset']->attr('start_date'))).': '.$replacee->name);
			}
		}

		return parent::_preCreateCheck($link);

	}//end _preCreateCheck()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody2()
	{
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b><?php echo translate('Name'); ?></b></td>

				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<?php
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE)) {
					$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
					if (is_array($root_url)) $root_url = $root_url[0];
				?>
			<tr>
				<td><b><?php echo translate('Replaces Cancellation') ?></b></td>

				<td><?php
					// get the parent recurring event asset
					$recurring_events = array_keys($recurring_events);
					$recurring_event = $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					if (isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
						$cancellation_href = htmlspecialchars(replace_query_string_vars(Array('SQ_CALENDAR_EVENT_ID' => $recurring_events[0], 'SQ_CALENDAR_EVENT_ACTION' => NULL)));
					} else {
						$cancellation_href = $root_url.'?a='.$recurring_events[0];
					}
					echo '<a href="'.$cancellation_href.'">'.$recurring_event->attr('name').'</a>';
				?></td>
			</tr>
			<?php
				}
			?>
			<tr>
				<td><b><?php echo translate('Start Date') ?></b></td>

				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('End Date') ?></b></td>

				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Duration') ?></b></td>

				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), TRUE);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('Description') ?></b></td>

				<td><?php echo $this->getKeywordReplacement('asset_attribute_description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody2()


	/**
	* Get bodycopy keyword replacement
	*
	* Provides a mechanism for providing keyword replacement functions by
	* looking up the function named 'getKeywordNameKeywordReplacement'
	* so an asset wishing to add support for 'best_ever' custom keyword
	* should implement a function getBestEverKeywordReplacement()
	* which returns a replacement string
	*
	* @param string	$keyword	Keyword to find a replacement for
	*
	* @return string
	* @access public
	*/
	function getKeywordReplacement($keyword)
	{
		$replacement = parent::getKeywordReplacement($keyword);
		$empty_replacement = (((string)$replacement) == '%'.$keyword.'%');

		// If we have not found an appropriate replacement, ask the parent, if it is a Calendar Multi Date Event asset
		if ($empty_replacement && (count($this->parent_links) > 0)) {
			// Only expecting one relevant link
			$parent_link = reset($this->parent_links);

			$parent_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($parent_link['majorid']);
			$replacement = $parent_asset->getKeywordReplacement($keyword);
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($parent_asset);
		}

		return $replacement;

	}//end getKeywordReplacement()


}//end class

?>
