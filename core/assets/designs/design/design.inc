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
					 SQ_LINK_UNITE     => Array('design_customisation' => 'M'),
					 SQ_LINK_USES      => Array('design_area'          => 'M'),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Moves a link from one place in this assets links order to another position
	*
	* @param int	$linkid		the link id of the link to move about
	* @param string $sort_order	the position in the links list that this link should take, if less than zero places at end of list
	*
	* @return boolean
	* @access public
	* @see Asset::moveLinkPos()
	*/
	function moveLinkPos($linkid, $sort_order=-1)
	{
		trigger_error('Links under Designs are system controlled and therefore cannot be moved', E_USER_WARNING);
		return false;
	}//end moveLinkPos()

	/**
	* Called whenever any type of link is changed, allows caching of objects, links and other stuff 
	* to occur but not cause integrity issues during script execution
	* returns true it it's successfull
	*
	* @return boolean
	* @access private
	*/
	function _linksUpdated()
	{
		if (!parent::_linksUpdated()) return false;
		if (!isset($this->_tmp['design_areas'])) return;
		foreach(array_keys($this->_tmp['design_areas']) as $id_name) unset($this->_tmp['design_areas'][$id_name]);
		return true;
	}//end _linksUpdated()

	/**
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
	}//end getDesignAreaLink()

	/**
	* Returns a design area for the given id_name
	*
	* @param string	$id_name		the id of the design area to get
	*
	* @return object Design_Area | null
	* @access public
	*/
	function &getDesignArea($id_name)
	{
		if (empty($this->_tmp['design_areas'][$id_name])) {
			if (!isset($this->_tmp['design_areas'])) $this->_tmp['design_areas'] = Array();
			$link = $this->getDesignAreaLink($id_name);
			if (is_null($link)) {
				$this->_tmp['design_areas'][$id_name] = null;
			} else {
				$this->_tmp['design_areas'][$id_name] = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['type_code']);
			}
		}
		return $this->_tmp['design_areas'][$id_name];
	}//end getDesignArea()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{

		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, $o, true);

				$id_name = $this->getAttribute('id_name');
				if ($id_name->processed) {
					return $this->create($link, $id_name->value);
				} else {
					return false;
				}
				break;

			default :
				$this->_tmp['regenerate_design'] = false;
				$this->_tmp['update_customisations'] = false;
				$processed = parent::processBackend($o, $link);
				if ($processed) {

					// regenerate the design, 
					// but don't do the customisations if they have yet to be updated 
					// (they will be regenerated during the update process anyway)
					if ($this->_tmp['regenerate_design'])     $this->generateDesignFile(!$this->_tmp['update_customisations']);
					if ($this->_tmp['update_customisations']) $this->_updateCustomisations();
					
				}// end if

				return $processed;

		}// end switch

	}// end processBackend()

	/**
	* Updates each of our customisations from ourselves
	*
	* @access private
	*/
	function _updateCustomisations()
	{
		$customisation_links = &$this->getLinks(SQ_LINK_UNITE, 'design_customisation', true, 'major', 'customisation');
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		foreach($customisation_links as $link) {
			$customisation = &$am->getAsset($link['minorid'], $link['type_code']);
			if (is_null($customisation)) continue;
			$customisation->updateFromParent($this);
		}// end foreach

	}// end _updateCustomisations()


	/**
	* Outputs the either as straight HTML or PHP code for the var, depending on the type
	*
	* @param string	$var	the name of the var to print out
	* @param Array	$args	array of any extra arguments that existed in the print command
	*
	* @access public
	*/
	function printVar($var, $args)
	{

		if (empty($args['escape']) || strtolower($args['escape']) == 'no') { 
			$escape_start = '';
			$escape_end = '';
		} else {
			$escape_start = 'addslashes(';
			$escape_end = ')';
		}

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
				<script language="javascript" type="text/javascript"  src="<?php echo $LIB_PATH; ?>/js/general.js"></script>
				<link rel="stylesheet" type="text/css"  href="<?php echo $LIB_PATH; ?>/css/general.css"></script>
				';

				break;

			case 'html_footer' :
				break;

			case 'site_name' :
			case 'page_title' :
				echo '"', ucwords(str_replace('_', ' ', $var)), '" NOT PROGRAMMED YET';
				break;

			case 'page_link' :
				echo '<?', 'php echo $ASSET->getHref(); ?', '>';
				break;

			case 'page_name' :
				echo '<?', 'php echo ', $escape_start, '$ASSET->name', $escape_end, '; ?', '>';
				break;


			case 'file_path' :
				echo '<?php echo $DATA_PATH;?>/'.$this->webFileSuffix($args['filename']);
				break;

			default :
				parent::printVar($var);

		}// end switch

	}//end printVar()


	/**
	* Paints out our design
	*
	* @param object Asset	$ASSET	the asset that we are going to print the body for
	*
	* @access public
	*/
	function paint(&$ASSET)
	{
		$DATA_PATH   = sq_web_path('data'); // this works because we know that all design areas are not going to be restricted
		$LIB_PATH    = sq_web_path('lib');
		// Now we just include the design file to get it to print.
		speed_check();
		require_once $this->data_path.'/design_file.php';
		speed_check();
	}//end paint()

	/**
	* (Re)Generates the design file
	*
	* @param bool	$do_customisations	Generate design files for any customisations
	*
	* @access public
	*/
	function generateDesignFile($do_customisations=true)
	{
		$edit_fns = $this->getEditFns();
		$edit_fns->createDesignFile($this);

		if ($do_customisations) {
			$customisation_links = &$this->getLinks(SQ_LINK_UNITE, 'design_customisation', true, 'major', 'customisation');
			$am = &$GLOBALS['SQ_SYSTEM']->am;
			foreach($customisation_links as $link) {
				$customisation = &$am->getAsset($link['minorid'], $link['type_code']);
				if (is_null($customisation)) continue;
				$customisation->generateDesignFile(true);
			}// end foreach
		}// end if

	}//end generateDesignFile()

	/**
	* Returns the file system path to the uploaded file
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	*/
	function filePath($filename)
	{
		return $this->data_path.'/files/'.$filename;
	}//end filePath()

	/**
	* Returns the path that has to be appended to the sq_web_path('base').'/__data' to
	* get access to the file for this design
	*
	* @param string	$filename	the filename to return the path to
	*
	* @return string
	* @access public
	*/
	function webFileSuffix($filename)
	{
		return $this->type().'/'.$this->id.'/files/'.$filename;
	}//end webFilesSuffix()

}//end class
?>
