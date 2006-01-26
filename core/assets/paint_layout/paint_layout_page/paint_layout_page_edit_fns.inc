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
* $Id: paint_layout_page_edit_fns.inc,v 1.6 2006/01/26 22:34:05 lwright Exp $
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
* @version $Revision: 1.6 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Paint_Layout_Page_Edit_Fns extends Page_Standard_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Paint_Layout_Page_Edit_Fns()
	{
		$this->Page_Standard_Edit_Fns();

	}//end constructor


	/**
	* Paint the current asset types that have been added
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('content');
		$folder =& $asset->getFolder('type_formats');

		$types = $asset->attr('types');
		if (empty($types) || !is_array($types)) {
			echo 'There are no types defined';
			return FALSE;
		}
		$type_names = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo(array_keys($types), 'name');
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header"><?php echo translate('asset_type'); ?></td>
				<td class="sq-backend-table-header"><?php echo translate('inherit_types'); ?></td>
				<?php
				if ($write_access) {
					?>
					<td class="sq-backend-table-header"><?php echo translate('remove_question'); ?></td>
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
							echo ($inherit) ? translate('yes') : translate('no');
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		$types = $asset->attr('types');
		$folder =& $asset->getFolder('type_formats');
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
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
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the backend outputter object
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
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


}//end class

?>
