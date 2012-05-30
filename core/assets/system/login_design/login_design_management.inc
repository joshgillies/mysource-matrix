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
* $Id: login_design_management.inc,v 1.8 2012/05/30 04:20:16 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Login_Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.8 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Login_Design_Management extends Asset_Management
{


	/**
	* Constructor
	*
	* @param Package_Manager	$pm	The package manager handling this asset
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'id_name'	=> Array(
										'default'	=> 'System Login Design',
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
		if (version_compare($current_version, '0.2', '<')) {
			// version 0.1 -> 0.2
			// Regenerate login designs to clear the cached design, because there is a potential XSS hole which was fixed years ago might still got cached
			pre_echo('UPGRADE LOGIN DESIGN AND EES LOGIN DESIGN');

			$design = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('login_design');
			$design_edit_fns = $design->getEditFns();
			$ees_design = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('ees_login_design');
			$ees_design_edit_fns = $ees_design->getEditFns();
			

			$design_edit_fns->parseAndProcessFile($design);
			$design->generateDesignFile();
			
			$ees_design_edit_fns->parseAndProcessFile($ees_design);
			$ees_design->generateDesignFile();
			
			pre_echo('LOGIN DESIGN AND EES LOGIN DESIGN UPGRADE COMPLETE - FROM VERSION 0.1');

		}//end if - upgrade to version 0.2
		
		return TRUE;

	}//end _upgrade()
	

}//end class
?>
