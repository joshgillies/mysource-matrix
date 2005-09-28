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
* $Id: data_source_graph.inc,v 1.1.1.1 2005/09/28 05:35:28 pgannavarapu Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Data_Source_Graph
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.1.1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Data_Source_Graph extends Asset
{

	/**
	* All allowed extensions for an image
	* @var Array(string)
	*/
	var $allowed_extensions = Array( 'png');


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Data_Source_Graph($assetid=0)
	{
		$this->_ser_attrs = true;
		$this->Asset($assetid);

	}//end constructor


	/**
	* Called by the design to print the body of this asset
	*
	* @return void
	* @access public
	*/
	function printBody()
	{
		$result = Array();
		$details = $this->attr('graph_details');
		$data_source = $GLOBALS['SQ_SYSTEM']->am->getLink($this->id, SQ_LINK_NOTICE, 'data_source', false, 'data_source_for_graph');
		if (!empty($data_source)) {
			$data_source_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($data_source['minorid']);
			$result = $data_source_asset->attr('result');
		}

		if (!empty($result)) {
			if (!empty($details)) {
				$x_axis = $details['x_axis'];
				$y_axis = $details['y_axis'];
				$type = $details['type'];

				include SQ_CONF_PEAR_PATH.'/Image/Graph.php';
				$Graph =& Image_Graph::factory('graph', Array(400, 300));
				$Plotarea =& $Graph->addNew('plotarea');
				$dataset =& Image_Graph::factory('dataset');

				foreach ($result as $record) {
					$co_ord = Array();
					foreach ($record as $key => $value) {
						if (($key == $x_axis) && is_numeric($value)) {
							$co_ord['x_axis'] = $value;
						} else if (($key == $y_axis) && is_numeric($value)) {
							$co_ord['y_axis'] = $value;
						}
					}
					if (!empty($co_ord)) {
						$dataset->addPoint($co_ord['x_axis'], $co_ord['y_axis']);
					}
				}
				if (empty($dataset->_data)) {
					echo '<br />One or more of the entered co-ordinates is not numeric!';
				} else {
					$Plot =& $Plotarea->addNew($type, &$dataset);
					require_once SQ_CORE_PACKAGE_PATH.'/files/image/image.inc';
					$Graph->done();
				}
			}
		} else {
			echo 'Sorry no results for a graph';
		}

	}//end printBody()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access private
	*/
	function _getName($short_name=false)
	{
		return ($this->attr('name'));

	}//end _getName()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_NOTICE	=> Array('data_source' => Array('card' => 1, 'exclusive' => false)),
			   );

	}//end _getAllowedLinks()


	/**
	* Return a human readable description of the passed link
	*
	* @param int	$linkid	the link ID of the link to describe
	*
	* @return string
	* @access public
	*/
	function describeLink($linkid)
	{
		$link = $GLOBALS['SQ_SYSTEM']->am->getLinkById($linkid);
		switch (strtolower($link['value'])) {
			case 'data_source_for_graph' :
				return ('Data Source Asset Linked To The Graph Asset '.$this->id);
			break;
			default :
				return parent::describeLink($linkid);
			break;
		}

	}//end describeLink()


}//end class

?>
