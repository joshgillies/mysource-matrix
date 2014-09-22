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
* $Id: paint_layout_page_edit_fns.inc,v 1.14 2012/08/30 01:09:09 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/page_templates/page_standard/page_standard_edit_fns.inc';
require_once SQ_CORE_PACKAGE_PATH.'/paint_layout/paint_layout_page/paint_layout_page.inc';

/**
* Paint_Layout_Page_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <brobertson@squiz.co.uk>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Page_Edit_Fns extends Page_Standard_Edit_Fns
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
	* Paint the current asset types that have been added
	*
	* @param Paint_Layout_Page	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintCurrentAssetTypes(Paint_Layout_Page $asset, Backend_Outputter $o, $prefix)
	{
		$write_access = $asset->writeAccess('content');
		$folder = $asset->getFolder('type_formats');

		$types = $asset->attr('types');
		if (empty($types) || !is_array($types)) {
			echo 'There are no types defined';
			return FALSE;
		}
		$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('Asset Type'); ?></td>

				<td class="sq-backend-table-header"><?php echo translate('Inherit Types'); ?></td>

				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header"><?php echo translate('Remove?'); ?></td>

					<?php
				}
				?>
			</tr>
			<?php
			foreach ($types as $type => $inherit) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
						<?php
						echo $type_names[$type];
						?>
					</td>
					<td class="sq-backend-table-cell">
						<?php
						if ($write_access) {
							check_box($prefix.'_inherit_types[]', $type, $inherit);
						} else {
							echo ($inherit) ? translate('Yes') : translate('No');


						}
						?>
					</td>
					<?php
					if ($write_access) {
						?>
						<td class="sq-backend-table-cell">
							<?php
							check_box($prefix.'_remove[]', $type, FALSE);
							?>
						</td>
						<?php
					}
					?>
				</tr>
				<?php
			}//end foreach $types
			?>
		</table>
		<?php
		return TRUE;

	}//end paintCurrentAssetTypes()


	/**
	* Process the current asset types that have been added
	*
	* @param Paint_Layout_Page	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processCurrentAssetTypes(Paint_Layout_Page $asset, Backend_Outputter $o, $prefix)
	{
		$types = $asset->attr('types');
		$folder = $asset->getFolder('type_formats');
		$formats = $asset->getFormats('type_formats');

		// Remove type format bodycopies
		$removed = array_get_index($_POST, $prefix.'_remove', Array());
		foreach ($removed as $type) {
			if (isset($types[$type])) {
				unset($types[$type]);

				if (in_array($type, $formats)) {
					// we also want to send the bodycopy to the trash
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($folder->id, SQ_LINK_TYPE_2, SQ_PAINT_LAYOUT_FORMAT_ASSET_TYPE, TRUE, $type);
					if (empty($link)) {
						trigger_localised_error('CORE0216', E_USER_WARNING);
					} else {
						// back up in your trash with the resurection
						$folder->deleteLink($link['linkid']);
					}
				}
			}
		}

		// update type format array
		if (!isset($_POST[$prefix.'_inherit_types'])) {
			$_POST[$prefix.'_inherit_types'] = Array();
		}

		for (reset($types); NULL !== ($k = key($types)); next($types)) {
			$types[$k] = (in_array($k, $_POST[$prefix.'_inherit_types'])) ? TRUE : FALSE;
		}
		$asset->setAttrValue('types', $types);

		return TRUE;

	}//end processCurrentAssetTypes()


	/**
	* Paint section to add an asset type
	*
	* @param Paint_Layout_Page	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function paintAddAssetType(Paint_Layout_Page $asset, Backend_Outputter $o, $prefix)
	{
		if (!$asset->writeAccess('content')) return FALSE;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));

		$type_options = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($types, 'name');
		asort($type_options);
		$type_options = Array('' => '- Select asset type -') + $type_options;
		combo_box($prefix.'_new_type', $type_options, FALSE, '');

	}//end paintAddAssetType()


	/**
	* Process the section to add a new format for an asset
	*
	* @param Paint_Layout_Page	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	public function processAddAssetType(Paint_Layout_Page $asset, Backend_Outputter $o, $prefix)
	{
		$ret = FALSE;

		if (isset($_POST[$prefix.'_new_type'])) {

			$type = trim($_POST[$prefix.'_new_type']);

			if (!empty($type)) {
				// add new type in the types list
				$types = $asset->attr('types');
				if (!isset($types[$type])) {
					$types[$type] = TRUE;
					$asset->setAttrValue('types', $types);
					$ret = TRUE;

					$ret = ($asset->createTypeFormatBodycopy($type));
				}
			}

		}

		return $ret;

	}//end processAddAssetType()


	/**
	* Paint the usage of this paint layout
	*
	* @param Paint_Layout_Page	$asset	the asset to which we belong
	* @param Backend_Outputter	$o		the backend outputter object
	* @param string				$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	public function paintUsage(Paint_Layout_Page $asset, Backend_Outputter $o, $prefix)
	{
		$o->openRaw();
		echo '<p><i>'.translate('This screen shows which parts of the asset tree this design is used in, by listing the highest-level assets to which it has been applied and the number of their children to which it has cascaded.').'</i></p>';

		echo '<div style="line-height: 160%">';
		$res = $GLOBALS['SQ_SYSTEM']->am->getAssetidsByLookupValue('paint_layout::%', $asset->id, TRUE);
		$res = $this->_getTreeTops($res);
		$singular = strtolower(translate('Child'));

		$plural = strtolower(translate('Children'));

		$paginate = (count($res) > 20);
		if ($paginate) {
			$current_page_no = array_get_index($_REQUEST, 'usage_page_no', 0);
			$total_pages = ceil(count($res) / 20);
			$res = array_slice($res, $current_page_no * 20, 20);
		}
		foreach ($res as $entry) {
			$count = $entry['child_count'];
			echo get_asset_tag_line($entry['assetid'], 'lookupValues');
			if (!empty($count)) {
				echo ' and '.$count.' '.(($count > 1) ? $plural : $singular);
			}
			echo '<br />';
		}
		echo '</div>';
		if ($paginate) {
			echo '<p>';
			if ($current_page_no > 0) {
				echo '<a href="'.htmlspecialchars(replace_query_string_vars(Array('usage_page_no' => $current_page_no-1))).'">'.translate('Previous Page').'</a> &nbsp; ';

			}
			echo sprintf(translate('Page %1$s of %2$s'), $current_page_no + 1, $total_pages);

			if ($current_page_no < ($total_pages-1)) {
				echo ' &nbsp; <a href="'.htmlspecialchars(replace_query_string_vars(Array('usage_page_no' => $current_page_no+1))).'">'.translate('Next Page').'</a> ';

			}
			echo '</p>';
		}
		$o->closeRaw();

	}//end paintUsage()


	/**
	* Get the "tree tops" in the supplied list of assets
	*
	* An asset is a "tree top" in a set of assets if none of its ancestors
	* in the asset tree are in the set with it.  Conversely if an asset's parent
	* or grandparent etc is in the set, then that asset is not a tree top.
	*
	* Returns an array indexed by treeid, where each value is another array
	* containing assetid and number of children of that assetid found in
	* the set.
	*
	* @param array	$assetids	The assetids we are analysing
	*
	* @return array
	* @access public
	*/
	protected function _getTreeTops(Array $treeids)
	{
		$all_treeids = Array();
		foreach ($treeids as $assetid => $asset_treeids) {
			foreach ($asset_treeids[0] as $asset_treeid) {
				if (($asset_treeid === 'override') || ($asset_treeid === 'not_override')) {
					continue;
				}
				$all_treeids[$asset_treeid] = $assetid;
			}
		}
		uksort($all_treeids, Array($this, '_strlenCmp'));
		$res = Array();
		foreach ($all_treeids as $treeid => $assetid) {
			$treeid_copy = $treeid;
			while (!empty($treeid_copy)) {
				if (isset($res[$treeid_copy])) {
					$res[$treeid_copy]['child_count']++;
					continue(2);
				}
				$treeid_copy = substr($treeid_copy, 0, -SQ_CONF_ASSET_TREE_SIZE);
			}
			$res[$treeid]['assetid'] = $assetid;
			$res[$treeid]['child_count'] = 0;
		}
		ksort($res); // put in treeid order
		return $res;

	}//end _getTreeTops()


	/**
	* Compare the lengths of two strings
	*
	* @param string	$a	First comparee
	* @param string	$b	Second comparee
	*
	* @return boolean
	* @access public
	*/
	protected function _strlenCmp($a, $b)
	{
		return strlen($a) > strlen($b);

	}//end _strlenCmp()


}//end class
?>
