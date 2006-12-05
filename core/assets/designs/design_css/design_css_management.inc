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
* $Id: design_css_management.inc,v 1.9 2006/12/05 22:56:41 emcdonald Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Css_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['id_name'] = Array(
									'added'		=> '0.0.1',
									'type'		=> 'text',
									'default'	=> '',
								 );
		$this->vars['cache_css'] = Array(
									'added'			=> '0.0.1',
									'type'			=> 'boolean',
									'default'		=> FALSE,
									'parameters'	=> Array('allow_empty' => FALSE),
								   );
		$this->vars['strip_whitespace'] = Array(
											'added'		=> '0.0.1',
											'type'		=> 'boolean',
											'default'	=> FALSE,
										  );

	}//end constructor


	/**
	* Upgrade this asset to the most current version
	*
	* @param string	$current_version	The version this asset is being upgraded from
	*
	* @return boolean
	* @access public
	*/
	function _upgrade($current_version)
	{
		if (!parent::_upgrade($current_version)) {
			return FALSE;
		}

		if (version_compare($current_version,'0.3','<')) {
			pre_echo('UPGRADING CSS FILE ASSET TYPE - FROM VERSION '.$current_version);
			// As of v0.3 all
			$css_assetids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_css');
			$GLOBALS['SQ_SYSTEM']->am->includeAsset('design_css');
			$asset =& new Design_CSS();
			foreach ($css_assetids as $assetid) {
				$asset->load($assetid);
				$cache_id = $asset->getCacheFileId();
				if (empty($cache_id)) {
					echo 'Creating cache file for CSS File #'.$assetid."\n";
					$asset->_createCacheFile();
				}
			}
			pre_echo('DONE UPGRADING CSS FILE ASSET TYPE');
		}

		return TRUE;

	}//end _upgrade()

}//end class
?>
