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
* $Id: design_css_management.inc,v 1.13.8.1 2012/07/23 01:21:27 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.13.8.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

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
											'added'			=> '0.0.1',
											'type'			=> 'boolean',
											'default'		=> FALSE,
											'parameters'	=> Array('allow_empty' => FALSE),
										  );
		$this->vars['static_url_versioning'] = Array(
												'added'			=> '0.4',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty' => FALSE),
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
		// first do things as normal
		$res = parent::_upgrade($current_version);

		if (version_compare($current_version, '0.4', '<')) {
			pre_echo('UPGRADING DESIGN CSS FROM v < 0.4');

			// get all design assets
			$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_css', TRUE, TRUE);
			foreach ($design_ids as $design_id => $type) {
				$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id, $type['type_code']);
				$design->setAttrValue('static_url_versioning', FALSE);
				$design->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
				unset($design);
			}
			
		}//end if
		
		if (version_compare($current_version, '0.5', '<')) {
			pre_echo('UPGRADING DESIGN CSS FROM v < 0.5');

			// regenerate all design assets
			$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design_css', TRUE, TRUE);
			foreach ($design_ids as $design_id => $type) {
				$vars = Array('assetid' => $design_id);
				$hh = $GLOBALS['SQ_SYSTEM']->getHipoHerder();
				$hh->freestyleHipo('hipo_job_regenerate_design', $vars);
			}
			
		}//end if
		
		return TRUE;

	}//end _upgrade(

}//end class
?>
