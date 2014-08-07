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
* $Id: page_multiple_page_edit_fns.inc,v 1.10 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page/page_edit_fns.inc';

/**
* Page_Multiple_Page_Edit_Fns
*
* Purpose
*
*
* @author  Darren McKee <dmckee@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Multiple_Page_Edit_Fns extends Page_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentPages(&$asset, &$o, $prefix)
	{
		$asset_ids = $asset->getPageIds();
		$asset_infos = $GLOBALS['SQ_SYSTEM']->am->getAssetInfo($asset_ids);
		$edit_ids = Array();
		foreach ($asset_ids as $id) {
			$edit_ids[$id] = 'contents';
		}
		$edit_links = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($edit_ids);
		$write_access = $asset->writeAccess('content');

		require_once SQ_LIB_PATH.'/html_form/html_form.inc';

		?>
		<script type="text/javascript">
			function sortOrderMoveDown(currentOrder) {
			  if (!document.getElementById) {
				  alert('<?php echo translate('cms_form_no_getelementbyid') ?>');
				  return;
			  }
			  // move a question up a row
			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder + 1));

			  // if there is no next element, then this is the last one; we shouldn't be running
			  if (!nextElement) return;

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml') ?>');
				  return;
			  }

			  // switch the question names
			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = nextElement.innerHTML;
			  nextElement.innerHTML = temp;

			  // switch the question types
			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_t' + currentOrder);
			  var nextElement = document.getElementById('<?php echo $prefix ?>_order_t' + (currentOrder + 1));

			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = nextElement.innerHTML;
			  nextElement.innerHTML = temp;



			  // switch the 'checked for deletion' parameters
			  temp = form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder+1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder+1) + ']'].checked = form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked = temp;

			  // switch the reorder values
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder+1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;

			}

			function sortOrderMoveUp(currentOrder) {
			  // move a question up a row
			  if (!document.getElementById) {
				  alert('<?php echo translate('cms_form_no_getelementbyid') ?>');
				  return;
			  }

			  if (currentOrder == 0) return;

			  var form = document.getElementById('main_form');

			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_o' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_o' + (currentOrder - 1));

			  if (!currentElement.innerHTML) {
				  alert('<?php echo translate('cms_form_no_innerhtml') ?>');
				  return;
			  }

			  // switch the question names
			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = prevElement.innerHTML;
			  prevElement.innerHTML = temp;

			  // switch the question types
			  var currentElement = document.getElementById('<?php echo $prefix ?>_order_t' + currentOrder);
			  var prevElement = document.getElementById('<?php echo $prefix ?>_order_t' + (currentOrder - 1));

			  var temp = currentElement.innerHTML;
			  currentElement.innerHTML = prevElement.innerHTML;
			  prevElement.innerHTML = temp;

			  // switch the delete checkboxes
			  // temp = form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder-1) + ']'].checked;
			  // form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked;
			  // form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked = temp;

			  temp = form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder-1) + ']'].checked;
			  form.elements['<?php echo $prefix ?>_delete_page[' + (currentOrder-1) + ']'].checked = form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked;
			  form.elements['<?php echo $prefix ?>_delete_page[' + currentOrder + ']'].checked = temp;

			  // switch the includes checkboxes
			  temp = form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + (currentOrder-1) + ']'].value = form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value;
			  form.elements['<?php echo $prefix ?>_order[reorder][' + currentOrder + ']'].value = temp;

			}
		</script>
			<table class="sq-backend-table">
				<tr>
					<th class="sq-backend-table-header"><?php echo translate('name'); ?></th>
					<th class="sq-backend-table-header"><?php echo translate('link'); ?></th>
					<?php
					if ($write_access) {
						?>
						<th class="sq-backend-table-header"><?php echo translate('delete'); ?>?</th>
						<th class="sq-backend-table-header"><?php echo translate('move'); ?>?</th>
						<?php
					}
					?>
				</tr>
				<?php
				if (empty($asset_ids)) {
					?>
					<tr>
						<td colspan="5" style="text-align: center;"><?php echo translate('currently_no_pages_to_display'); ?></td>
					</tr>
					<?php
				}
				$i=0;
				$sort_order = $asset->getPageLinks();
				foreach ($sort_order as $sort_id => $asset_info) {
				?>
					<tr>
						<td class="sq-backend-table-cell">
							<span name="<?php echo $prefix.'_order_o'.$sort_id ?>" id="<?php echo 	$prefix.'_order_o'.$sort_id ?>"><?php echo $asset_infos[$asset_info['minorid']]['name']; ?></span>
							<?php hidden_field($prefix.'_order[reorder]['.$sort_id.']', $asset_info['minorid']); ?>
						</td>
						<td class="sq-backend-table-cell">
							<span name="<?php echo $prefix.'_order_t'.$sort_id ?>" id="<?php echo 	$prefix.'_order_t'.$sort_id ?>"><a href="<?php echo $edit_links[$asset_info['minorid']]; ?>"><?php echo translate('contents'); ?></a></span>
						</td>
						<?php
						if ($write_access) {
							?>
							<td class="sq-backend-table-cell">
								<?php check_box($prefix.'_delete_page['.$sort_id.']'); ?>
							</td>
							<td align="center" width="100" class="sq-backend-table-cell">
								<?php
								if ($i != 0) {
									?><a href="#" onclick="sortOrderMoveUp(<?php echo $sort_id ?>); return false;"><script type="text/javascript">sq_print_icon("<?php echo sq_web_path('lib').'/web/images/icons/up_arrow.png' ?>", "16", "16", "Move Up");</script></a><?php
								}
								if ($i != count($sort_order) -1) {
									?><a href="#" onclick="sortOrderMoveDown(<?php echo $sort_id ?>); return false;"><img src="<?php echo sq_web_path('lib').'/web/images/icons/down_arrow.png' ?>" width="16" height="16" border="0"alt="Move Down" /></a><?php
								}
								?>
							</td>
						<?php
					$i++;
					}
					?>
				</tr>
			<?php
			}//end foreach
			?>
		</table>
		<?php

		if ($write_access) {
			?>
			<table>
				<tr>
				<td>
					<?php echo translate('create_new_page'); ?>
				</td>
				<td>
					<?php text_box($prefix.'_new_page', '', '50'); ?>
				</td>
				</tr>
			</table>
			<?php
		}

		return TRUE;

	}//end paintCurrentPages()


	/**
	* Paint the interface for creating containers in the bodycopy
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentPages(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');

		$am = $GLOBALS['SQ_SYSTEM']->am;

		if (isset($_POST[$prefix.'_order']['reorder'])) {
			$new_sort_order = $_POST[$prefix.'_order']['reorder'];
			$new_sort_order_flip = array_flip($new_sort_order);
			// Get the old sort_order
			$sort_order = $asset->getPageLinks();
			$new_paths = Array();
			$new_paths_next = Array();
			$switch = TRUE;
			foreach ($sort_order as $num => $asset_info) {
				if ($num != $new_sort_order_flip[$asset_info['minorid']]) {
					$asset->sortOrder($asset_info['linkid'], $sort_order[$new_sort_order_flip[$asset_info['minorid']]]['sort_order'], $switch=FALSE);
				}
			}
		}

		if (isset($_POST[$prefix.'_delete_page'])) {
			if (isset($_POST[$prefix.'_order']['reorder'])) {
				$sort_order_array = $_POST[$prefix.'_order']['reorder'];
			}

			// Now we have sort_order_array to match, match them
			$delete_list = Array();
			foreach ($sort_order_array as $key => $id) {
				if (array_get_index($_POST[$prefix.'_delete_page'],$key,NULL)) {
					$delete_list[$id] = '1';
				}
			}
		}

		if ($write_access && isset($_POST[$prefix.'_delete_page'])) {
			// Set the following flag, so the updating of lookups doesnt cause dramas
			$asset->_tmp['deleting_links'] = TRUE;
			foreach ($delete_list as $id => $flag) {
				$doomed_link = $am->getLinkByAsset($asset->id, $id);
				$am->moveLink($doomed_link['linkid'], $am->getSystemAssetid('trash_folder'), SQ_LINK_TYPE_2, '0');
				$deleted = $am->getAsset($doomed_link['minorid']);
				$deleted->saveWebPaths(Array());
			}
		}
		// Restore the flag set above
		$asset->_tmp['deleting_links'] = FALSE;

		if ($write_access && isset($_POST[$prefix.'_new_page']) && !empty($_POST[$prefix.'_new_page'])) {
			$asset->createChildPage($_POST[$prefix.'_new_page']);
		}

		$asset->_tmp['no_sort_order_paths'] = TRUE;
		$asset->linksUpdated();

		return $write_access;

	}//end processCurrentPages()


}//end class

?>
