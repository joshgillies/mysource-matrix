<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: page_sub_asset_listing_edit_fns.inc,v 1.1 2003/10/10 04:23:12 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../../page/page_edit_fns.inc';

/**
* Page_Sub_Asset_Listing_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Sub_Asset_Listing_Edit_Fns extends Page_Edit_Fns
{

	/**
	* Constructor
	*
	*/
	function Page_Sub_Asset_Listing_Edit_Fns()
	{
		$this->Page_Edit_Fns();
		$this->static_screens['details']['force_unlock'] = false;

	}//end constructor
	

	/**
	* paints a link to the default format
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLink(&$asset, &$o, $prefix)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, 'default_format');
		if (empty($link)) return false;

		$info = Array($link['minorid'] => 'contents');
		$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
		echo '<a href="'.$href[$link['minorid']].'">Edit the Default Format</a>';

		return true;

	} // end paintDefaultFormatLink()


	/**
	* does nothing (yet)
	*
	* @return boolean
	* @access public
	*/
	function processDefaultFormatLink()
	{
		return true;

	} // end processDefaultFormatLink()


	/**
	* processes the current asset types
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentAssetTypes(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		// do a check to see if any of the bodycopies have been removed manually
		$customised = $asset->attr('customised');
		if (!empty($customised)) {
			$deleted = false;
			foreach ($customised as $type) {
				$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
				if (empty($link)) {
					$deleted = true;
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
			if ($deleted) {
				$asset->setAttrValue('customised', $customised);
				$asset->saveAttributes();
			}
		}

		$types = $asset->attr('types');
		if (empty($types)) {
			echo 'There are no Types Defined';
			return false;
		}
		
		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header" width="33%">Asset Type</td>
			<td class="sq-backend-table-header" width="33%">Edit Format</td>
			<td class="sq-backend-table-header" width="33%">Available Keywords</td>
			<td class="sq-backend-table-header" width="33%">Customise</td>
			<td class="sq-backend-table-header" width="33%">Remove?</td>
		</tr>
			<?php
			foreach ($types as $type) {
				?>
				<tr>
					<td class="sq-backend-table-cell">
					
					<?php
					echo ucwords(str_replace('_',' ', $type));
					?>
					
					</td>
					<td class="sq-backend-table-cell">
					<?php
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', true, $type);
						$is_customised = false;
						if (!empty($link)) {
							$is_customised = true;
							$info = Array($link['minorid'] => 'contents');
							$href = $GLOBALS['SQ_SYSTEM']->am->getAssetBackendHref($info);
							
							$content = '<a href="'.$href[$link['minorid']].'">'.ucwords(str_replace('_',' ', $type)).' Format</a>';
						} else {
							$content = 'Default Format';
						}
						echo $content;
					?>
					</td>
					<td class="sq-backend-table-cell">
						<a href="#" onClick="Javascript: var win = window.open('<?php echo sq_web_path('data')?>/asset_types/page_sub_asset_listing/popups/keywords.php?type_code=<?php echo $type?>', 'keywords', 'toolbar=0,menubar=0,location=0,status=0,scrollbars=1,resizable=1,width=580,height=520'); return false;">Available Keywords</a>
					</td>
					<td class="sq-backend-table-cell">
					
					<?php
					check_box($prefix.'_customise[]', $type, $is_customised);
					?>
					
					</td>
					<td class="sq-backend-table-cell">
					
					<?php
					check_box($prefix.'_remove[]', $type, false);
					?>
					
					</td>
				</tr>
				<?php
			}
		?>
		</table>
		<?php

	}// end processCurrentAssetTypes()


	/**
	* paints the current asset types that have been added
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentAssetTypes(&$asset, &$o, $prefix)
	{

		$customised = $asset->attr('customised');
		$types = $asset->attr('types');

		// we are looking to see if any types have been removed
		if (isset($_POST[$prefix.'_remove'])) {
			foreach ($_POST[$prefix.'_remove'] as $type) {
				if (in_array($type, $types)) {
					$key = array_search($type, $types, true);
					unset($types[$key]);

					// remove from the customise list if its in here
					if (in_array($type, $customised)) {
						$key = array_search($type, $customised, true);
						unset($customised[$key]);

						// we also want to send the bodycopy to the trash
						$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, $type);
						if (empty($link)) {
							trigger_error('Format for type '.$type.' not found', E_USER_WARNING);
						}
						// back up in your trash with the resurection
						$asset->deleteLink($link['linkid']);
					}
				}
			}
			$asset->setAttrValue('types', $types);
 		}
		if (isset($_POST[$prefix.'_customise'])) {
			
			if (empty($_POST[$prefix.'_customise'])) return false;
			
			foreach ($_POST[$prefix.'_customise'] as $type) {
				if (in_array($type, $customised)) continue;
				
				// dickhead check
				if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;
				
				$customised[] = $type;

				$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
				$GLOBALS['SQ_SYSTEM']->am->includeAsset('bodycopy');
				$bodycopy = new Bodycopy();
				$copy_link = Array('asset' => &$asset, 'value' => $type, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

				$bodycopy->setAttrValue('name', ucwords(str_replace('_',' ', $type)).' Format');

				if (!$bodycopy->create($copy_link)) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					return false;
				}
				$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			
			} // end foreach
		} // end if
		if (!empty($customised)) {
			foreach ($customised as $type) {
				if (!isset($_POST[$prefix.'_customise']) || !in_array($type, $_POST[$prefix.'_customise'])) {

					// another dickhead check
					if (isset($_POST[$prefix.'_remove']) && in_array($type, $_POST[$prefix.'_remove'])) continue;
					
					$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_TYPE_2, 'bodycopy', false, $type);
					$asset->deleteLink($link['linkid']);
					$key = array_search($type, $customised, true);
					unset($customised[$key]);
				}
			}
		}
		$asset->setAttrValue('customised', $customised);
		$asset->saveAttributes();
	
	}// end paintCurrentAssetTypes()


	/**
	* paints section to add an asset type
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddAssetType(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess()) return false;

		$types = $GLOBALS['SQ_SYSTEM']->am->getTypeList();
		$types = array_diff($types, $asset->attr('types'));
		asort($types);
		
		$uc_types = Array();
		foreach ($types as $type) {
			$uc_types[$type] = ucwords(str_replace('_',' ', $type));
		}
		combo_box($prefix.'_types', $uc_types, true, '', 6);

	}// end paintAddAssetType()


	/**
	* paints section to add a new format for an asset
	*
	* @param object Form				&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddAssetType(&$asset, &$o, $prefix)
	{
		if (isset($_POST[$prefix.'_types'])) {
			$new_types = $_POST[$prefix.'_types'];
			if (empty($new_types)) return false;

			$types = $asset->attr('types');
			
			foreach ($new_types as $type) {
				if (in_array($type, $types)) return false;
				$types[] = $type;
			}

			$asset->setAttrValue('types', $types);
			$asset->saveAttributes();
		}

	} // end paintAddFormat()
	
}//end class

?>