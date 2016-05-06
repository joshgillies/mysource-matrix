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
* $Id: poll_question_edit_fns.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Poll_Question_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Robert Howard <rhoward@squiz.net>
* @author  David Heppell <dheppell@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Poll_Question_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Poll_Question_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = FALSE;

	}//end constructor


	/**
	* Paint the poll options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPollOptions(&$asset, &$o, $prefix)
	{

		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		// include script for re-ordering questions
		?>
		<script language="JavaScript">
			function sortOrderMoveDown(currentOrder) {
			  if (!document.getElementById) {
				  alert(js_translate('Browser cannot support script - Get Element By ID is not defined'));

				  return;
			  }

			  var form = document.getElementById('main_form');

			  if (!document.getElementById('<?php echo $prefix ?>_option_text' + (currentOrder + 1))) return;

			  // switch the text-box values
			  var currentText = document.getElementById('<?php echo $prefix ?>_option_text' + currentOrder);
			  var nextText = document.getElementById('<?php echo $prefix ?>_option_text' + (currentOrder + 1));
			  temp = currentText.value;
			  currentText.value = nextText.value;
			  nextText.value = temp;

			  // switch the votes values around
			  var currentVotes = document.getElementById('<?php echo $prefix ?>_option_votes' + currentOrder);
			  var prevVotes = document.getElementById('<?php echo $prefix ?>_option_votes' + (currentOrder + 1));
			  temp = currentVotes.innerHTML;
			  currentVotes.innerHTML = prevVotes.innerHTML;
			  prevVotes.innerHTML = temp;

			  // switch the delete checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

			  // switch the clear votes checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[clear_votes][' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[clear_votes][' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[clear_votes][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[clear_votes][' + currentOrder + ']'].checked = temp;

			  // switch the reorder values
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;
			}

			function sortOrderMoveUp(currentOrder) {
			  if (!document.getElementById) {
				  alert(js_translate('Browser cannot support script - Get Element By ID is not defined'));

				  return;
			  }

			  var form = document.getElementById('main_form');

			  if (currentOrder == 0) return;

			  // switch the text-box values
			  var currentText = document.getElementById('<?php echo $prefix ?>_option_text' + currentOrder);
			  var prevText = document.getElementById('<?php echo $prefix ?>_option_text' + (currentOrder - 1));
			  temp = currentText.value;
			  currentText.value = prevText.value;
			  prevText.value = temp;

			  // switch the votes values around
			  var currentVotes = document.getElementById('<?php echo $prefix ?>_option_votes' + currentOrder);
			  var prevVotes = document.getElementById('<?php echo $prefix ?>_option_votes' + (currentOrder - 1));
			  temp = currentVotes.innerHTML;
			  currentVotes.innerHTML = prevVotes.innerHTML;
			  prevVotes.innerHTML = temp;

			  // switch the delete checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

			   // switch the clear votes checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[clear_votes][' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[clear_votes][' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[clear_votes][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[clear_votes][' + currentOrder + ']'].checked = temp;

			  // switch the reorder values
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;
			}
		</script>

		<table class="sq-backend-table">
					<tr>
						<td class="sq-backend-table-header">
							Poll Option
						</td>
						<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;text-align:center;">
							Votes
						</td>
						<?php

						if ($admin_access) {

							?>
							<td style="font-weight: bold;text-align:center;" width="100" class="sq-backend-table-header" align="center" >Delete?</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;text-align:center;">Move</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;text-align:center;">Clear Votes?</td>
							<?php
						}

						?>
					</tr>
					<?php

				// Grab array of options
				$poll_options = $asset->attr('options');

				if (is_null($poll_options)) return FALSE;

				$ii = 0;
				foreach ($poll_options as $sort_id => $sort_data) {
					?>
						<tr>
							<!-- Option Text -->
							<td class="sq-backend-table-cell">
								<?php
									if ($admin_access) {
										text_box($prefix.'_option_text'.$sort_id,$sort_data['option_text'], 30);
									} else {
										echo $sort_data['option_text'];
									}
									?>
								<?php
									hidden_field($prefix.'_order[reorder]['.$sort_id.']', $sort_id);
								?>
							</td>
							<!-- No. of Votes -->
							<td align="center" width="100" class="sq-backend-table-cell">
								<span id="<?php echo $prefix.'_option_votes'.$sort_id; ?>">
								<?php
									echo $sort_data['votes'];
								?>
								</span>
							</td>
							<?php
								if ($admin_access) {
									?>
									<td align="center" width="100" class="sq-backend-table-cell">
										<?php
										check_box($prefix.'_order[delete]['.$sort_id.']');
										?>
									</td>
									<td align="center" width="100" class="sq-backend-table-cell">
										<?php
										if ($ii != 0) {
											?><a href="#" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a><?php
										}

										if ($ii != count($poll_options) -1) {
											?><a href="#" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><img src="<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>" width="16" height="16" border="0"alt="Move Down" /></a><?php
										}
										?>
									</td>
									<td align="center" width="100" class="sq-backend-table-cell">
										<?php
										check_box($prefix.'_order[clear_votes]['.$sort_id.']');
										?>
									</td>
									<?php
								}//end if admin access
							?>
						</tr>
						<?php
						$ii++;
					}//end foreach data
				?>
				</table>
		<?php
		return TRUE;

	}//end paintPollOptions()


	/**
	* Process options for this poll
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPollOptions(&$asset, &$o, $prefix)
	{

		$prefix = $asset->getPrefix();
		$options = $asset->attr('options');
		$option_updated = FALSE;

		// Re-order and re-name the list
		if (isset($_POST[$prefix.'_order']['reorder'])) {
			$reorder_array = $_POST[$prefix.'_order']['reorder'];
			$newArray = Array();
			foreach ($reorder_array as $reorder_index => $reorder_data) {
				$options[$reorder_data]['option_text'] = $_POST[$prefix.'_option_text'.$reorder_index];
				array_push($newArray, $options[$reorder_data]);
				$option_updated = TRUE;
			}
			$options = $newArray;
		}

		// Delete any items flagged for deletion
		if (isset($_POST[$prefix.'_order']['delete'])) {
			foreach (array_keys($options) as $option_index ) {
				if (isset($_POST[$prefix.'_order']['delete'][$option_index])) {
					if ($_POST[$prefix.'_order']['delete'][$option_index] == '1') {
						$options[$option_index] = NULL;
						unset($options[$option_index]);
						$options = array_values($options);
					}
				}
				$option_updated = TRUE;
			}
		}

		// Clears all the votes based on specific items flagged
		if (isset($_POST[$prefix.'_order']['clear_votes'])) {
			foreach (array_keys($options) as $option_index ) {
				if (isset($_POST[$prefix.'_order']['clear_votes'][$option_index])) {
					if ($_POST[$prefix.'_order']['clear_votes'][$option_index] == '1') {
						$options[$option_index]['votes'] = 0;

						$options = array_values($options);
					}
				}
				$option_updated = TRUE;
			}
		}

		if ($option_updated) $asset->updateOptions($options);
		return TRUE;


	}//end processPollOptions()


	/**
	* Paint the add new poll options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddPollOptions(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		if ($admin_access) {
			combo_box($prefix.'_no_options', Array( '','1','2','3','4','5','6','7','8','9','10' ), FALSE, '');
		}

	}//end paintAddPollOptions()


	/**
	* Process adding an option for this poll
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddPollOptions(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();
		$options = $asset->attr('options');

		if (isset($_POST[$prefix.'_no_options'])) {
			for ($ii = 0; $ii < $_POST[$prefix.'_no_options']; $ii++) {
				$new_option = Array('option_text' => 'Poll Option Text #'.($ii + 1), 'votes' => 0);
				array_push($options, $new_option);
			}
			$asset->setAttrValue('options', $options);
		}
		return TRUE;

	}//end processAddPollOptions()


	/**
	* Paint the add new poll options
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDeleteUsers(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$users_array = Array('','all' => 'Delete All Users' );

		// construct delete options array
		$voters = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($asset->id, NULL, SQ_LINK_NOTICE, NULL, 'major', TRUE);

		if ($admin_access) {
			if (empty($voters)) {
				?><em><?php echo translate('No voting records exist for this question'); ?></em><?php

			} else {
				foreach ($voters as $voter) {
					$voter_info = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo(Array($voter['minorid']));
					$users_array[$voter['minorid']] = $voter_info[$voter['minorid']]['name'];
				}

				combo_box($prefix.'options', $users_array, FALSE, '');
			}
		} else {
			?><em><?php echo translate('You require Admin Permission for this question to be able to delete users\' voting records'); ?></em><?php

		}

	}//end paintDeleteUsers()


	/**
	* Processes removing the list of users who have already voted
	*
	* @param object	&$asset	the asset whose details were changing
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDeleteUsers(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();

		if (!empty($_POST[$prefix.'options'])) {
			$other_assetid = ($_POST[$prefix.'options'] == 'all') ? NULL : $_POST[$prefix.'options'];
			$voter_links = $GLOBALS['SQ_SYSTEM']->am->getShadowLinkByAsset($asset->id, $other_assetid, SQ_LINK_NOTICE, NULL, 'major', TRUE);

			foreach ($voter_links as $voter_link) {
				$GLOBALS['SQ_SYSTEM']->am->deleteShadowAssetLink($voter_link['linkid']);
			}
		}

		return TRUE;

	}//end processDeleteUsers()


}//end class


?>