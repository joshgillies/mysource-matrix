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
* $Id: poll_question_edit_fns.inc,v 1.1 2004/12/23 04:04:42 dheppell Exp $
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
* @version $Revision: 1.1 $
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

	}//end constructor


	/**
	* Paint the poll options
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintPollOptions(&$asset, &$o, $prefix)
	{

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();

		// include script for re-ordering questions
		?>
		<script language="JavaScript">
			function sortOrderMoveDown(currentOrder) {
			  if (!document.getElementById) {
				  alert('Browser cannot support script - Get Element By ID is not defined');
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

			  // switch the delete checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder+1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

			  // switch the reorder values
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;
			}

			function sortOrderMoveUp(currentOrder) {
			  if (!document.getElementById) {
				  alert('Browser cannot support script - Get Element By ID is not defined');
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

			  // switch the delete checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_order[delete][' + currentOrder + ']'].checked = temp;

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
						<?php

						if ($admin_access) {

							?>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td>
							<td align="center" width="100" class="sq-backend-table-header" style="font-weight: bold;">Move</td>
							<?php

						}

						?>
					</tr>
					<?php

				// Grab array of options
				$poll_options = $asset->attr('options');

				if (is_null($poll_options)) {
					return false;
				}

				$ii = 0;
				foreach ($poll_options as $sort_id => $sort_data) {
					?>
						<tr>
							<td class="sq-backend-table-cell">
								<!-- sort_id => assetid -->
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
		return true;

	}//end paintPollOptions()


	/**
	* Process options for this poll
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processPollOptions(&$asset, &$o, $prefix)
	{

		$prefix = $asset->getPrefix();
		$options = $asset->attr('options');
		$option_updated = false;

		// Re-order and re-name the list
		if (isset($_POST[$prefix . '_order']['reorder'])) {
			$reorder_array = $_POST[$prefix . '_order']['reorder'];
			$newArray = Array();
			foreach ($reorder_array as $reorder_index => $reorder_data) {
				$options[$reorder_data]['option_text'] = $_POST[$prefix . '_option_text'.$reorder_index];
				array_push($newArray, $options[$reorder_data]);
				$option_updated = true;
			}
			$options = $newArray;
		}

		// Delete any items flagged for deletion
		if (isset($_POST[$prefix . '_order']['delete'])) {
			foreach (array_keys($options) as $option_index ) {
				if (isset($_POST[$prefix . '_order']['delete'][$option_index])) {
					if ($_POST[$prefix . '_order']['delete'][$option_index] == '1') {
						$options[$option_index] = null;
						unset($options[$option_index]);
						$options = array_values($options);
					}
				}
				$option_updated = true;
			}
		}

		if ($option_updated) {
			$asset->updateOptions($options);
		}
		return true;


	}//end processPollOptions()


	/**
	* Paint the add new poll options
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddPollOptions(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		if ($admin_access) {
			combo_box($prefix.'_no_options', Array( '','1','2','3','4','5','6','7','8','9','10' ), false, '');
		}

	}//end paintAddPollOptions()


	/**
	* Process adding an option for this poll
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddPollOptions(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();
		$options = $asset->attr('options');

		if (isset($_POST[$prefix . '_no_options'])) {
			for ($ii = 0; $ii < $_POST[$prefix . '_no_options']; $ii++) {
				$new_option = Array('option_text' => 'Poll Option Text #'.($ii + 1), 'votes' => 0);
				array_push($options, $new_option);
			}
			$asset->setAttrValue('options', $options);
		}
		return true;

	}//end processAddPollOptions()

	/**
	* Paint the add new poll options
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDeleteUsers(&$asset, &$o, $prefix)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$admin_access = $asset->writeAccess('attributes');
		$prefix = $asset->getPrefix();
		$users_array = Array('','Delete All Users' );

		// construct delete options array
		if (is_array($asset->attr('voters'))) {
			$users_array = array_merge($users_array, array_keys($asset->attr('voters')));
		}


		if ($admin_access) {
			combo_box($prefix.'options', $users_array, false, '');
		}

	}//end paintDeleteUsers()



	/**
	* Processes removing the list of users who have already voted
	*
	* @param object Form_Section        &$asset     the asset whose details were changing
	* @param object	Backend_Outputter   &$o         the outputter class
	* @param string                     $prefix     prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDeleteUsers(&$asset, &$o, $prefix)
	{
		$prefix = $asset->getPrefix();
		$voters = $asset->attr('voters');

		if (isset($_POST[$prefix . 'options']) == 'Delete All Users') {
			$voters = null;
		} else {
			$voters[$_POST[$prefix . 'options']] == null;
		}

		$asset->setAttrValue('voters', $voters);

	}//end processDeleteUsers()


}//end class


?>