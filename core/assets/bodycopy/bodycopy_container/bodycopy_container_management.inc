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
* $Id: bodycopy_container_management.inc,v 1.10 2006/12/05 05:39:49 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Container_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Container_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Bodycopy_Container_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars = Array(
						'name'			=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'text',
											'default'	=> '',
										   ),
						'attributes'	=> Array(
											'added'		=> '0.0.1',
											'type'		=> 'serialise',
											'default'	=> Array(),
										   ),
						'shadow_links'	=> Array(
											'added'			=> '0.3',
											'type'			=> 'serialise',
											'default'		=> Array(),
											'description'	=> 'Array holding the assetids belonging to shadow assets that are used within the content. Needed because they cannot be notice linked like normal assets can.',
										   ),
					  );

	}//end constructor


	/**
	* Upgrade
	*
	* @param string	$current_version	the version of the asset that is currenly installed
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) return FALSE;

		$asset_name = strtoupper($this->_pm->getTypeInfo($this->getAssetType(), 'name'));

		if (version_compare($current_version, '0.5', '<')) {
			// version 0.3 -> 0.5 (0.4 was a botched version)
			// escaping of "<?php", "<?", etc. now happens in content_type::paint(), but we need to
			// regenerate each bodycopy_container's content file so that we catch any nasties that might
			// already be present that we need to get rid of.
			// ignore cases where <?php has already been escaped to &lt;?php
			pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.5');
			// get all 'bodycopy_container' assets (or their descendants)
			$containers = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE);
			pre_echo('Upgrading '.count($containers).' assets...');
			foreach ($containers as $assetid) {
				$container =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

				// reasonably critical upgrade - report any assets that didn't make it
				if (is_null($container)) {
					pre_echo('Could get asset #'.$assetid);
					continue;
				}

				// regenerate the bodycopy_container's content file
				$container_edit_fns =& $container->getEditFns();
				if (!$container_edit_fns->generateContentFile($container)) {
					pre_echo('Could not generate content file for asset #'.$assetid);
				}

				unset($container_edit_fns);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($container);
			}
			pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.5');

		}//end version 0.5 upgrade

		return TRUE;

	}//end _upgrade()


}//end class

?>
