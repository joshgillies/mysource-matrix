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
* $Id: calendar_event_single.inc,v 1.4 2004/07/29 05:17:10 tbarrett Exp $
* $Name: not supported by cvs2svn $
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
* @version $Revision: 1.4 $ - 1.0
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

	}//end Calendar_Event_Single()


	/**
	* Create this asset
	*
	* @param Array	&$link	information used to create the initial link
	*
	* @return mixed int or boolean	the link ID or the create status if no link
	* @access public
	* @see Asset::create()
	*/
	function create(&$link) 
	{
		$parent = &$link['asset'];
		if ($parent->type() == 'calendar_event_cancellation') {
			if (!$this->attr('name')) {
				$this->setAttrValue('name', 'Rescheduled for ' . $this->getFriendlyStart());
			} else {
				$good_words = array_intersect(explode(' ', strtolower($this->attr('name'))), Array('changed', 'moved', 'rescheduled'));
				if (empty($good_words)) {
					$this->setAttrValue('name', 'Rescheduled: ' . $this->attr('name'));
				}
			}
		}
		return parent::create($link);

	}//end create()


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		include_once SQ_FUDGE_PATH.'/general/datetime.inc';
		?>
		<table border="0" cellspacing="2" cellpadding="2">
			<tr>
				<td><b>Event Name</b></td>
				<td><?php echo $this->attr('name'); ?></td>
			</tr>
			<?php 
				if ($recurring_events = $GLOBALS['SQ_SYSTEM']->am->getParents($this->id, 'calendar_event_recurring', false)) {
					$root_url = array_values($GLOBALS['SQ_SYSTEM']->am->getRootURL());
					if (is_array($root_url)) $root_url = $root_url[0];
				?>
			<tr>
				<td><b>Replaces cancellation</b></td>
				<td><?php 
					// get the parent recurring event asset
					$recurring_event = &$GLOBALS['SQ_SYSTEM']->am->getAsset($recurring_events[0]);
					echo '<a href="' . $root_url . '?a=' . $recurring_events[0] . '">' . $recurring_event->attr('name') . '</a>';
				?></td>
			</tr>
			<?php
				}
			?>
			<tr>
				<td><b>Start Date</b></td>
				<td><?php echo $this->getFriendlyStart(); ?></td>
			</tr>
			<tr>
				<td><b>End Date</b></td>
				<td><?php echo $this->getFriendlyEnd(); ?></td>
			</tr>
			<tr>
				<td><b>Duration</b></td>
				<td><?php
						// give useful info if no duration - easy_time_total returns empty string
						$total_time = easy_time_total($this->getDuration(), true);
						echo (empty($total_time) ? 'N/A' : $total_time);
					?></td>
			</tr>
			<tr>
				<td><b>Description</b></td>
				<td><?php echo $this->attr('description'); ?></td>
			</tr>
		</table>
		<?php

	}//end printBody()


}//end class

?>