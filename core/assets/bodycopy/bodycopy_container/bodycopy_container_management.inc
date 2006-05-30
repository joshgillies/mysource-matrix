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
* $Id: bodycopy_container_management.inc,v 1.6.2.3 2006/05/30 05:19:06 rhoward Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Bodycopy_Container_Management
*
* @see Asset_Management
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.6.2.3 $
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

		$this->vars = Array('name'			=> Array(
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
