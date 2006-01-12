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
* $Id: calendar_event_single.inc,v 1.13.2.1 2006/01/12 01:18:04 skim Exp $
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
* @version $Revision: 1.13.2.1 $
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
	* @access private
	* @return boolean
	*/
	function _preCreateCheck(&$link)
	{
		if (!parent::_preCreateCheck($link)) return false;

		$parent = &$link['asset'];
		if ($parent->type() == 'calendar_event_cancellation') {
			if (!$this->attr('name')) {
				$this->setAttrValue('name', str_replace('Cancelled', 'Rescheduled from ', $parent->attr('name')));
			} else {
				$good_words = array_intersect(explode(' ', strtolower($this->attr('name'))), Array('changed', 'moved', 'rescheduled'));
				if (empty($good_words)) {
					$this->setAttrValue('name', 'Rescheduled: '.$this->attr('name'));
				}
			}
		}

		return true;

	}//end _preCreateCheck()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		require_once SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b><?php echo translate('name'); ?></b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<?php
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false)) {
					$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
					if (is_array($root_url)) $root_url = $root_url[0];
				?>
			<tr>
				<td><b><?php echo translate('cal_event_replaces_cancellation') ?></b></td>
				<td><?php
					// get the parent recurring event asset
					$recurring_events = array_keys($recurring_events);
					$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					if (isset($_REQUEST['SQ_CALENDAR_EVENT_ID'])) {
						$cancellation_href = replace_query_string_vars(Array('SQ_CALENDAR_EVENT_ID' => $recurring_events[0], 'SQ_CALENDAR_EVENT_ACTION' => null));
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
						$total_time = easy_time_total($this->getDuration(), true);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b><?php echo translate('description') ?></b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>