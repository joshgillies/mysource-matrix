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
* $Id: design_management.inc,v 1.23 2011/02/01 04:50:20 mhaidar Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Management
*
* @see Asset_Management
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Revision: 1.23 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function __construct(Package_Manager $pm)
	{
		Asset_Management::__construct($pm);

		$this->vars = Array(
						'id_name' 			=> Array(
												'added'		=> '0.0.1',
												'type'		=> 'text',
												'default'	=> '',
								 			   ),
						'wysiwyg_classes'	=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Use saved classes in the wysiwyg dropdown list',
											   ),
						'div_classes'		=> Array(
												'added'			=> '0.2',
												'type'			=> 'serialise',
												'default'		=> Array(),
												'description'	=> 'Use saved classes in the DIV container properties class dropdown',
											   ),
						'limbo_no_frames'	=> Array(
												'added'			=> '0.1',
												'type'			=> 'boolean',
												'default'		=> FALSE,
												'parameters'    => Array (
                                                                    'allow_empty'   => FALSE,
                                                                   ),
												'description'	=> 'LIMBO no frame options for each design, not using frames while inside LIMBO mode',
											   ),
						'static_url_versioning' => Array(
												'added'			=> '0.3',
												'type'			=> 'boolean',
												'default'		=> TRUE,
												'parameters'	=> Array('allow_empty' => FALSE),
										  )
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

		if (version_compare($current_version, '0.3', '<')) {
			pre_echo('UPGRADING DESIGN FROM v < 0.3');

			// get all design assets
			$design_ids = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetids('design', TRUE, TRUE);
			foreach ($design_ids as $design_id => $type) {
				$design = $GLOBALS['SQ_SYSTEM']->am->getAsset($design_id, $type['type_code']);
				$design->setAttrValue('static_url_versioning', FALSE);
				$design->saveAttributes();
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($design);
				unset($design);
			}
			
		}//end if
		return TRUE;

	}//end _upgrade()


}//end class


?>
