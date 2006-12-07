<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: calendar_event_single.inc,v 1.17 2006/12/07 05:51:50 bcaldwell Exp $
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
* @version $Revision: 1.17 $
* @package MySource_Matrix_Packages
* @subpackage calendar
*/
class Calendar_Event_Single extends Calendar_Event
{


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
		if (is_a($link['asset'], 'calendar_event_cancellation')) {
			$replacee_link = $GLOBALS['SQ_SYSTEM']->am->getLink($link['asset']->id, NULL, '', TRUE, NULL, 'minor');
			$replacee =& $GLOBALS['SQ_SYSTEM']->am->getAsset($replacee_link['majorid']);
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
				<td><b><?php echo translate('name'); ?></b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<?php
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', FALSE)) {
					$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
					if (is_array($root_url)) $root_url = $root_url[0];
				?>
			<tr>
				<td><b><?php echo translate('cal_event_replaces_cancellation') ?></b></td>
				<td><?php
					// get the parent recurring event asset
					$recurring_events = array_keys($recurring_events);
					$recurring_event =& $GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					if (isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
						$cancellation_href = replace_query_string_vars(Array('SQ_CALENDAR_EVENT_ID' => $recurring_events[0], 'SQ_CALENDAR_EVENT_ACTION' => NULL));
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
				<td><b><?php echo translate('start_date') ?></b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('end_date') ?></b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('duration') ?></b></td>
				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), TRUE);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('description') ?></b></td>
				<td><?php echo $this->getKeywordReplacement('asset_attribute_description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>