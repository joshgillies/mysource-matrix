<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: ipb_poll_edit_fns.inc,v 1.6 2012/08/30 00:58:55 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Ipb_Poll_Edit_Fns
*
* Purpose
*
*
* @author  Scott Kim <skim@squiz.net>
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage ipb
*/
class Ipb_Poll_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Ipb_Poll_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

		$this->static_screens = Array(
									'details'		=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
									'dependants'	=> Array(
														'name'			=> 'Dependants',
														'force_unlock'	=> TRUE,
														'lock_type'		=> 'none',
													   ),
								);

	}//end constructor


	/**
	* Paint the summary information of all polls
	*
	* @param object	&$asset	Ipb_Poll object
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPoll(&$asset, &$o, $prefix)
	{

		if ($asset->_suffix == '0') {
			$ipb_bridge =& $GLOBALS['SQ_SYSTEM']->am->getAsset($asset->_ipbid, 'ipb_bridge');
			$ipb =& $ipb_bridge->getIpbConn();
			$list_of_polls = $ipb->getListOfPolls();

			if (empty($list_of_polls)) {
				$o->openField(translate('List of Polls'));

					echo translate('Currently there is no polls in IPB');

				$o->closeField();
			} else {
				$o->openSection(translate('Summary'));

					$o->openField(translate('Number of Polls'));

						echo count($list_of_polls);
					$o->closeField();

					$o->openField(translate('List of Polls'));

						$ipb =& $ipb_bridge->getIpbConn();
						?>
						<table class="sq-backend-table">
							<tr>
								<th style="width: 45%"><?php echo translate('Question'); ?></th>

								<th style="width: 50%"><?php echo translate('Asset'); ?></th>

								<th style="width: 15%"><?php echo translate('State'); ?></th>

							</tr>
						<?php
						foreach ($list_of_polls as $poll) {
							$data = $ipb->getTopicInfo($ipb_bridge->id.':'.'t_'.$poll['tid']);
						?>
							<tr>
								<td style="font-weight: bold;"><?php echo $poll['poll_question']; ?></td>
								<td><?php echo  get_asset_tag_line($asset->_ipbid.':'.'o_'.$poll['pid']); ?></td>
								<td><?php echo  $data['poll_state']; ?></td>
							</tr>
						<?php
						}
						echo '</table>';
					$o->closeField();
				$o->closeSection();
			}

		} else {

			$o->openSection('Details');
				$poll_question = $asset->attr('poll_question');
				$start_date = date('g:i:s D j M Y', $asset->attr('start_date'));

				$started_by = get_asset_tag_line($asset->_ipbid.':'.'u_'.$asset->attr('starter_id'));
				$forum_tag_line = get_asset_tag_line($asset->_ipbid.':'.'f_'.$asset->attr('forum_id'));
				$total_votes = $asset->attr('votes');
				$fields = Array(
							translate('Poll Question')	=> $poll_question,

							translate('Start Date')		=> $start_date,

							translate('Started by')		=> $started_by,

							translate('Forum')			=> $forum_tag_line,

							translate('Total votes')	=> $total_votes,

						  );
				foreach ($fields as $key => $value) {
					$o->openField($key);
						echo $value;
					$o->closeField();
				}

				$o->openField('Results');
					$poll_data = @unserialize(stripslashes($asset->attr('choices')));
					if ($poll_data === FALSE) {
						echo '<b>'.translate('Can not retrieve the result data. Serialization error occurred.').'</b>';

					} else {
						$total_votes = $asset->attr('votes');
						?>
						<table class="sq-backend-table">
							<tr>
								<th style="width: 40%"><?php echo translate('Option'); ?></th>

								<th style="width: 30%"><?php echo translate('Votes'); ?></th>

								<th style="width: 30%"><?php echo translate('Percentage'); ?></th>

							</tr>
						<?php
						foreach ($poll_data as $data) {
						?>
							<tr>
								<td><?php echo $data[1]; ?></td>
								<td><?php echo $data[2]; ?></td>
								<td><?php echo ($data[2]==0) ? 0 : ($data[2]/$total_votes) * 100; ?> %</td>
							</tr>
						<?php
						}
						echo '</table>';
					}

				$o->closeField();

			$o->closeSection();
		}//end else

		return FALSE;

	}//end paintPoll()


	/**
	* Processes the interface for displaying the summary information
	*
	* It does nothing
	*
	* @param object	&$asset	Ipb_Poll object
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPoll(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processPoll()


}//end class

?>
