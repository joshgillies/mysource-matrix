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
* $Id: asset_edit_fns.inc,v 1.33 2004/02/05 05:58:28 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Asset_Edit_Fns
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Asset_Edit_Fns extends MySource_Object
{

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $static_screens = Array('details'			=> Array(
														'name'			=> 'Details',
														'force_unlock'	=> true,
														'lock_type'		=> 'attributes',
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

		foreach($paths as $path) {
			if ($wa) text_box($prefix.'_paths[]', $path, 25);
			else echo $path;
			echo '<br/>';
		}

		if ($wa) text_box($prefix.'_paths[]', '', 25);

		return $wa;

	}// end paintPaths()


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
		if (!$asset->saveWebPaths(array_values($_POST[$prefix.'_paths']))) return false;
		
		//// START HIPO PROCESSING ////
		$hh = &$GLOBALS['SQ_SYSTEM']->getHipoHerder();
		$vars = Array('assetids' => Array($asset->id));
		$hh->queueHipo('hipo_job_update_lookups', $vars);
		////  END HIPO PROCESSING  ////

		return true;

	}// end processPaths()


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
			echo 'There are no URLs associated with this Asset';
		} else {
			foreach($urls as $data) {
				if ($data['http'])  echo 'http://'.$data['url'], '<br/>';
				if ($data['https']) echo 'https://'.$data['url'], '<br/>';
			}
		}

		if ($asset->force_secure) {
			echo '<br>';
			$o->note('Force Secure (see Settings screen) is set on this asset, so if you think that there should other URLs registered you may need to check that the site(s) that this asset is has allowed the https protocol');
		}

	}// end paintURLs()


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
				if ($weightingData['weighting'] > 0) {
					echo $weightingData['weighting'];
				} else {
					echo "Not included in searching.";
				}
				if ($default) {
					echo '&nbsp;<b>[default]</b>';
				}
			} else {
				?><input type="checkbox" value="1" name="<?php echo $prefix; ?>weightings[<?php echo $component;?>][default]" <?php if ($default) echo 'checked';?>> default&nbsp;&nbsp;
				<?php

				if ($default) {
					echo "&nbsp;&nbsp;Weight: ";
					echo $weightingData['weighting'];
					$o->addHiddenField($prefix. "weightings[$component][weighting]", $weightingData['weighting']);
					$o->addHiddenField($prefix. "include[$component][weighting]", ($weightingData['weighting'] > 0)?'1':'0');
				} else {
					combo_box($prefix. "include[$component]", Array("0" => "Do not include in searches", "1" => "Include in searches"), false, ($weightingData['weighting'] > 0)?'1':'0');
					if ($weightingData['weighting'] > 0) {
						echo "&nbsp;&nbsp;Weight: ";
						text_box ($prefix. "weightings[$component][weighting]", $weightingData['weighting'], 3);
					} else {
						$o->addHiddenField($prefix. "weightings[$component][weighting]", $weightingData['weighting']);
					}
				}
			}

			$o->closeField();
		}
		return true;
	}
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
		$im =& $GLOBALS['SQ_SYSTEM']->getIndexingManager();

		foreach ($weightings as $component => $weightingData) {
			$weighting =& $weightings[$component]['weighting'];
			if (isset($included[$component])) {
				if ($included[$component]) {
					$weighting = max("1", $weighting);
				} else {
					$weighting = '0';
				}	
			}
			if(!isset($weightings[$component]['default']))
				$weightings[$component]['default'] = '0';
		}

		$owner->saveComponentWeightings($weightings);
		$im->changed($owner->id);
		return true;
	}
}//end class

?>