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
* $Id: asset_edit_fns.inc,v 1.47.2.4 2006/06/20 00:58:26 skim Exp $
*
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.47.2.4 $
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var array
	*/
	var $static_screens = Array(
							'details'			=> Array(
													'name'			=> 'Details',
													'force_unlock'	=> true,
													'lock_type'		=> 'menu',
												   ),
							'permissions'		=> Array(
													'name'			=> 'Permissions',
													'force_unlock'	=> false,
													'lock_type'		=> 'permissions',
												   ),
							'workflow'			=> Array(
													'name'			=> 'Workflow',
													'force_unlock'	=> true,
													'lock_type'		=> 'workflow',
												   ),
							'metadataSchemas'	=> Array(
													'name'			=> 'Metadata Schemas',
													'force_unlock'	=> true,
													'lock_type'		=> 'metadata',
												   ),
							'metadata'			=> Array(
													'name'			=> 'Metadata',
													'force_unlock'	=> true,
													'lock_type'		=> 'metadata',
												   ),
							'dependants'		=> Array(
													'name'			=> 'Dependants',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'linking'			=> Array(
													'name'			=> 'Linking',
													'force_unlock'	=> true,
													'lock_type'		=> 'links',
												   ),
							'settings'			=> Array(
													'name'			=> 'Settings',
													'force_unlock'	=> true,
													'lock_type'		=> 'all',
												   ),
							'history'			=> Array(
													'name'			=> 'History',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'logs'				=> Array(
													'name'			=> 'Logs',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'preview'			=> Array(
													'name'			=> 'Preview',
													'force_unlock'	=> true,
													'lock_type'		=> 'none',
												   ),
							'lookupValues'		=> Array(
													'name'			=> 'Lookup Settings',
													'force_unlock'	=> false,
													'lock_type'		=> 'lookupValues',
												   ),
						  );


	/**
	* Constructor
	*
	*/
	function Asset_Edit_Fns()
	{
		$this->MySource_Object();

	}//end constructor


	/**
	* Paint the interface for the asset paths
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintPaths(&$asset, &$o, $prefix)
	{

		$wa = $asset->writeAccess('lookups');

		$paths = $asset->getWebPaths();

		foreach ($paths as $path) {
			if ($wa) {
				text_box($prefix.'_paths[]', $path, 25);
			} else {
				echo $path;
			}
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

		return $wa;

	}//end paintPaths()


	/**
	* Processes the Paths posted from the paintPaths fn above
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processPaths(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_paths'])) return false;
		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']))) {
			return false;
		}

		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);

		return true;

	}//end processPaths()


	/**
	* Paint the interface for the asset paths
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintURLs(&$asset, &$o, $prefix)
	{
		$urls = $asset->getLookups();
		if (empty($urls)) {
			echo translate('no_associated_urls');
		} else {
			usort($urls, create_function('$a, $b', 'return strcmp($a[\'url\'], $b[\'url\']);'));
			foreach ($urls as $data) {
				if ($data['http']) {
					echo 'http://'.$data['url'], '<br/>';
				}
				if ($data['https']) {
					echo 'https://'.$data['url'], '<br/>';
				}
			}
		}

		if ($asset->force_secure) {
			echo '<br>';
			$o->note(translate('force_secure_set_on_asset'));
		}

	}//end paintURLs()


	/**
	* Paint the interface for setting weights
	*
	* @param object Asset				&$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return booleam
	* @access public
	*/
	function paintWeights(&$owner, &$o, $prefix)
	{
		$weightings = $owner->getWeightings();
		$wa = $owner->writeAccess('weights');

		foreach ($weightings as $component => $weightingData) {
			$o->openField($weightingData['description']);
			$default = $weightingData['default'];
			if (!$wa) {
				if ($weightingData['weight'] > 0) {
					echo $weightingData['weight'];
				} else {
					echo translate('not_included_in_searching');
				}
				if ($default) {
					echo '&nbsp;<b>['.strtolower(translate('default')).']</b>';
				}
			} else {
				?><input type="checkbox" value="1" name="<?php echo $prefix; ?>weightings[<?php echo $component;?>][default]" <?php if ($default) echo 'checked';?>> <?php echo translate('default'); ?>&nbsp;&nbsp;
				<?php

				if ($default) {
					echo '&nbsp;&nbsp;Weight: ';
					echo $weightingData['weight'];
					$o->addHiddenField($prefix. "weightings[$component][weight]", $weightingData['weight']);
					$o->addHiddenField($prefix. "include[$component][weight]", ($weightingData['weight'] > 0)?'1':'0');
				} else {
					combo_box($prefix. "include[$component]", Array('0' => 'Do not include in searches', '1' => 'Include in searches'), false, ($weightingData['weight'] > 0)?'1':'0');
					if ($weightingData['weight'] > 0) {
						echo '&nbsp;&nbsp;Weight: ';
						text_box($prefix. "weightings[$component][weight]", $weightingData['weight'], 3);
					} else {
						$o->addHiddenField($prefix. "weightings[$component][weight]", $weightingData['weight']);
					}
				}
			}

			$o->closeField();
		}
		return true;

	}//end paintWeights()


	/**
	* Process the weights posted in paintWeights
	*
	* @param object Asset				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processWeights(&$owner, &$o, $prefix)
	{
		$weightings = $_POST[$prefix.'weightings'];
		$included   = $_POST[$prefix.'include'];
		$sm = &$GLOBALS['SQ_SYSTEM']->am->getSystemAsset('search_manager');
		if (!$sm) return false;

		foreach ($weightings as $component => $weightingData) {
			$weighting =& $weightings[$component]['weight'];
			if (isset($included[$component])) {
				if ($included[$component]) {
					$weighting = max('1', $weighting);
				} else {
					$weighting = '0';
				}
			}
			if (!isset($weightings[$component]['default'])) {
				$weightings[$component]['default'] = '0';
			}
		}

		$owner->saveComponentWeightings($weightings);
		$owner->informChanges();
		return true;

	}//end processWeights()


	/**
	* Starts workflow by writing out all workflow arrays to the workflow table for the passed assetID
	*
	* @param int    $assetid        the ID of the asset to check workflow completness for
	*
	* @return boolean
	* @access public
	*/
	function isWorkflowComplete(&$asset)
	{
		return $GLOBALS['SQ_SYSTEM']->wm->isWorkflowComplete($asset->id);

	}//end isWorkflowComplete()


	/**
	* Paint a generic interface for editing assets that are notice linked to this asset
	*
	* @param object Asset	$asset		the asset whose interface we are painting
	* @param Array			$link		the current link
	* @param Array			$type_codes type_codes to pass to asset_finder()
	* @param string			$field_name field_name to pass to asset_finder()
	*
	* @return boolean true if editable interface printed
	* @access public
	*/
	function paintNoticeLinkedAsset(&$asset, $link_value, $type_codes=Array(), $field_name='')
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);
		$selected = (empty($link)) ? 0 : $link['minorid'];

		if (empty($field_name))	$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));

		if (!$asset->writeAccess('links')) {
			echo '<i>';
			if (!$selected) {
				echo 'None';
			} else {
				$linked_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($selected, '', true);
				echo $linked_asset->name, ' (#', $linked_asset->id, ')';
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($linked_asset);
			}
			echo '</i>';
			return false;
		}

		if (SQ_IN_LIMBO) {
			asset_finder($field_name, $selected, $type_codes, '', false);
		} else {
			asset_finder($field_name, $selected, $type_codes);
		}

		return true;

	}//end paintNoticeLinkedAsset()


	/**
	* Process a generic interface for editing assets that are notice linked to this asset
	*
	* @param object Asset	$asset		the asset whose interface we are painting
	* @param string			$link_value	the value to put on the link
	* @param string			$field_name	the name used for the asset finder
	*
	* @return boolean	whether a change was made or not
	* @access public
	*/
	function processNoticeLinkedAsset(&$asset, $link_value, $field_name='')
	{
		if (empty($field_name))	$field_name = $asset->getPrefix().'_'.str_replace(' ', '_', strtolower($link_value));
		if (!isset($_POST[$field_name])) return false;

		$link = $GLOBALS['SQ_SYSTEM']->am->getLink($asset->id, SQ_LINK_NOTICE, '', false, $link_value);

		// if there has been no change, then there is nothing to do
		if (!empty($link) && $link['minorid'] == $_POST[$field_name]['assetid']) {
			return false;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$success = true;

		// if there is a current link, we need to delete it
		if (!empty($link)) {
			$success = (bool) $asset->deleteLink($link['linkid']);
		}

		if ($success && !empty($_POST[$field_name]['assetid'])) {
			$selected_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$field_name]['assetid']);
			if (!is_null($selected_asset)) {
				$success = (bool) $asset->createLink($selected_asset, SQ_LINK_NOTICE, $link_value);
			}
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction($success ? 'COMMIT' : 'ROLLBACK');
		return $success;

	}//end processNoticeLinkedAsset()


}//end class

?>
