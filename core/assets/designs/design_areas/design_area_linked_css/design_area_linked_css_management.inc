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
* $Id: design_area_linked_css_management.inc,v 1.7 2007/06/27 02:00:54 hnguyen Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Design_Area_Linked_Css_Management
*
* @see Asset_Management
*
* @author  Avi Miller <avi.miller@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Linked_Css_Management extends Asset_Management
{


	/**
	* Constructor
	*
	*/
	function Design_Area_Linked_Css_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['tag_type'] = Array(
									'added'			=> '0.0.1',
									'type'			=> 'selection',
									'parameters'	=> Array(
														'options'		=> Array(
																			'link_tag'		=> 'Link Tag',
																			'import_tag'	=> '@import Tag',
																		   ),
														'multiple'		=> FALSE,
														'allow_empty'	=> FALSE,
													   ),
									'default'		=> 'link_tag',
								  );

		$this->vars['media'] = Array(
								'added'			=> '0.0.1',
								'type'			=> 'selection',
								'parameters'	=> Array(
													'options'		=> Array(
																		'all'			=> 'All',
																		'aural'			=> 'Aural',
																		'braille'		=> 'Braille',
																		'embossed'		=> 'Embossed',
																		'handheld'		=> 'Handheld',
																		'print'			=> 'Print',
																		'projection'	=> 'Projection',
																		'screen'		=> 'Screen',
																		'tty'			=> 'TTY',
																		'tv'			=> 'TV',
																	   ),
													'multiple'		=> FALSE,
													'allow_empty'	=> FALSE,
												   ),
								'default'		=> 'all',
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

        if (version_compare($current_version, '0.2', '<')) {

        	pre_echo('STARTING '.$asset_name.' UPGRADE - TO VERSION 0.2');
            // get all 'design_area_linked_css' assets (or their descendants)

            // Upgrade code - Should regenerate and update the design file for all design with linked_css design area.

            $linked_css_design_areas = $GLOBALS['SQ_SYSTEM']->am->getTypeAssetIds($this->getAssetType(), FALSE);
            pre_echo('Upgrading '.count($linked_css_design_areas).' assets...');

            // Try to regenerate all the design through each of the linkedcss da.
            foreach ($linked_css_design_areas as $assetid) {
            	$linked_css_da =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);

                if (is_null($linked_css_da)) {
                    pre_echo('Could not get asset #'.$assetid);
                    continue;
                }//end if

                // Get the design that this linked_css da is used.
                $designs =& $linked_css_da->getDesigns();
                foreach ($designs as $design) {
                	// regenerate the design file
                	$design->generateDesignFile(FALSE);
                	$design->_updateCustomisations();
                }//end foreach

				unset($designs);
				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($linked_css_da);

            }//end foreach

            pre_echo($asset_name.' UPGRADE COMPLETE - FROM VERSION '.$current_version.' TO 0.2');

        }//end if version 0.1 upgrade

        return TRUE;

    }//end _upgrade()


}//end class
?>
