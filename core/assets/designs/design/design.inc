<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design_area/design_area.inc';

/**
* Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class Design extends Design_Area
{
	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Design($assetid=0) 
	{
		$this->Design_Area($assetid);
	}

	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() 
	{
		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array('design_area' => 'M'),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


	/*
	* Returns the link information for the design area with the given id_name or all design areas
	*
	* @param string	$id_name		the id of the design area to get, if blank returns links for all design areas
	*
	* @return Array | null
	* @access public
	*/
	function getDesignAreaLink($id_name='') 
	{
		if ($id_name) {
			return @$this->getLink(SQ_LINK_USES, 'design_area', false, $id_name);
		} else {
			return @$this->getLinks(SQ_LINK_USES, 'design_area', false);
		}
	}//end getDesignArea()

	/*
	* Returns a design area for the given id_name
	*
	* @param string	$id_name		the id of the design area to get
	*
	* @return Design_Area | null
	* @access public
	*/
	function &getDesignArea($id_name) 
	{
		if (empty($this->_tmp['design_areas'][$id_name])) {
			if (!isset($this->_tmp['design_areas'])) $this->_tmp['design_areas'] = Array();
			$link = $this->getDesignAreaLink($id_name);
			if ($link === null) {
				$this->_tmp['design_areas'][$id_name] = null;
			} else {
				$this->_tmp['design_areas'][$id_name] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code'], true);
			}
		}
		return $this->_tmp['design_areas'][$id_name];
	}//end getDesignArea()


	/*
	* processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend() 
	{
		// set this
		$this->tmp['need_file_parse'] = false;
		// now process as usual
		parent::processBackend();
		// and if anything was set that gives us cause for a re-parse, execute the parse
		if($this->tmp['need_file_parse']) {
			trigger_error("Parsing Not Done", E_USER_NOTICE);
		}// end switch

	}// end processBackend()

	/*
	* Outputs the var either as straight HTML or PHP code, depending on the type
	*
	* @param string	$var	the name of the var to print out
	*
	* @access public
	*/
	function printVar($var) 
	{
		switch($var) {
			case 'html_header' :
				echo '
				<!--   <?php echo SQ_CONF_SYSTEM_NAME; ?> 
					  Content © Copyright <?php echo SQ_CONF_SYSTEM_OWNER; ?> 
					Running <?php echo SQ_SYSTEM_LONG_NAME; ?> - <?php echo SQ_SYSTEM_URL; ?> 
				   Developed by Squiz  - http://www.squiz.net 
				  © Copyright <?php echo date(\'Y\') ?>. All rights reserved.
				  Page generated: <?php $time = getdate(); echo $time[\'mday\'], \' \', $time[\'month\'], \' \', $time[\'year\']; ?>
				-->
				<script language="javascript" type="text/javascript"  src="<?php echo SQ_WWW_LIB_DIR; ?>/js/general.js"></script>
				<link rel="stylesheet" type="text/css"  href="<?php echo SQ_WWW_LIB_DIR; ?>/css/general.css"></script>
				';

				break;

			case 'html_footer' :
				break;

			case 'file_prefix' :
				echo '<?php echo $FILE_PREFIX;?>';
				break;

			default :
				parent::printVar($var);
		}// end switch

	}//end printVar()


	/*
	* Paints out our design
	*
	* @param object Asset	$ASSET	the asset that we are going to print the body for
	*
	* @access public
	*/
	function paint(&$ASSET) 
	{
		// this works because we know that all design areas are not going to be restricted
		$DATA_PATH   = SQ_WWW_BASE_DIR.'/__data';
		$FILE_PREFIX = $DATA_PATH.'/'.$this->type().'/'.$this->id.'/files';

		// Now we just include the design file to get it to print.
		speed_check();
		require_once $this->_data_path.'/design_file.php';
		speed_check();
	}//end paint()


}//end class
?>
