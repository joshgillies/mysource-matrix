<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: design_area_asset_lineage.inc,v 1.1 2003/10/13 09:53:10 brobertson Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design_Area_Asset_Lineage
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Design_Area_Asset_Lineage extends Design_Area
{

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var			the name of the var to print out
	* @param Array	$args			array of any extra arguments that existed in the print command
	* @param string	$value_prefix	PHP code to wrap put before the value before outputting it
	* @param string	$value_suffix	PHP code to wrap put after  the value before outputting it
	*
	* @access public
	*/
	function printVar($var, $args, $value_prefix='', $value_suffix='')
	{

		switch($var) {
			case 'assetid' :
				echo '<', '?php echo $ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'assetid\']; ?', '>';
				break;

			case 'asset_link' :
				echo '<', '?php echo $ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'protocol\'].\'://\'.$ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'url\']; ?', '>';
				break;

			case 'asset_short_name' :
			case 'asset_name' :
				echo '<', '?php echo ', $this->_escapeVar($value_prefix.'$ASSET_LINEAGE[$'.$this->attr('id_name').'_i][\'short_name\']'.$value_suffix, $args), '; ?', '>';
				break;

			default :
				parent::printVar($var, $args);

		}// end switch

	}//end printVar()


}//end class
?>
