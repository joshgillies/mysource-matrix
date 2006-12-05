<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: data_source_edit_fns.inc,v 1.5 2006/12/05 05:53:09 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';

/**
* Data_Source_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Data_Source_Edit_Fns()
	{
		$this->Asset_Edit_Fns();

	}//end constructor


	/**
	* Paints the interface for entering the name of the shadow asset
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintShadow(&$asset, &$o, $prefix)
	{
		$name = $asset->attr('shadow_name');
		$wa = $asset->writeAccess('attributes');
		if ($wa) {
			text_box('shadow_name', !empty($name) ? $name : '', 80);
			return TRUE;
		} else if (!empty($name)) {
			echo $name;
			return FALSE;
		}

	}//end paintShadow()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processShadow(&$asset, &$o, $prefix)
	{
		$new_name = NULL;
		$name = $asset->attr('shadow_name');
		if (!empty($_POST['shadow_name'])) {
			$new_name = $_POST['shadow_name'];
		}
		if ($name != $new_name) {
			$res = $asset->setAttrValue('shadow_name', $new_name);
			return TRUE;
		} else {
			return FALSE;
		}

	}//end processShadow()


	/**
	* Paints the interface for entering the name of the shadow asset
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return void
	* @access public
	*/
	function paintKeywords(&$asset, &$o, $prefix)
	{
		$shadow_asset =& $asset->getAsset($asset->id.':0');
		if (is_null($shadow_asset)) {
			echo '<strong>'.translate('data_source_no_keywords').'</strong>';
			return;
		}
		$keywords = $shadow_asset->getAvailableKeywords();
		$wa = $asset->writeAccess('attributes');
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('keyword');?></th>
				<th><?php echo translate('description');?></th>
			</tr>
			<?php
			foreach ($keywords as $key => $desc) {
				// all the keywords other than the shadow asset name are to be displayed for the user to choose the
				// keywords or a combination of keywords and strings to set the shadow asset name
				if ($key == 'data_source_record_set_shadow_asset_name') {
					unset($keywords[$key]);
					continue;
				} else {
					if (strpos($key, 'data_source_record_set') === FALSE) {
						unset($keywords[$key]);
						continue;
					}
				}
				?>
				<tr>
					<td><?php echo '%'.$key.'%';?> </td>
					<td><?php echo $desc; ?> </td>
				</tr>
			<?php
			}
		?>
		</table>
		<?php
		if ($wa) {
			return TRUE;
		} else {
			return FALSE;
		}

	}//end paintKeywords()


	/**
	* Processes the interface for entering and changin the where clause.
	*
	* @param object	&$asset	the search page asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processKeywords(&$asset, &$o, $prefix)
	{
		return FALSE;

	}//end processKeywords()


	/**
	* Find out if there are any tables selected or not
	*
	* @param object	&$asset	The asset that might use the files
	* @param string	$prefix	the prefix for this asset
	*
	* @return boolean
	* @access protected
	*/
	function resultSetExists(&$asset, $prefix)
	{
		$result = $asset->getResultSet();
		if (empty($result)) {
			return FALSE;
		} else {
			return TRUE;
		}

	}//end resultSetExists()


}//end class



?>
