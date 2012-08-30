<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: workflow_stream.inc,v 1.2 2012/08/30 01:09:17 ewang Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Workflow_Stream
*
* Purpose
*    The Workflow Stream asset provides Workflow Schema assets with the ability
*    to offer alternative workflow paths on a single schema. For example, an
*    "urgent" stream could offer an accelerated workflow path
*
*    Each Workflow Schema shall have at least one (default) stream, and zero or
*    more additional streams. Only users with Admin permission to the asset will
*    be able to start the workflow with a stream other than the default.
*
*    Normal rules of started workflows apply: once a certain stream has been
*    started, any changes to the workflow or workflow streams will not apply
*    until the current workflow is finished.
*
* @author  Luke Wright <lwright@squiz.com.au>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Workflow_Stream extends Asset
{


	/**
	* Constructor
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		return Array(
				SQ_LINK_TYPE_1	=> Array(),
				SQ_LINK_TYPE_2	=> Array(
									'workflow_step'				=> Array(
																	'card'		=> 'M',
																	'exclusive'	=> FALSE,
																   ),
								   ),
				SQ_LINK_TYPE_3	=> Array(),
				SQ_LINK_NOTICE	=> Array(),
			   );

	}//end _getAllowedLinks()


	/**
	* Returns the name of the asset
	*
	* @return string
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Prepares for linking by checking that the link values are valid for this link
	*
	* This function will be called if this asset is the major or minor party in the link, so the side_of_link flag
	* should be checked to work out what side of the link this asset is on. The return value should indicate
	* if any of the link fields were changed.
	*
	* @param Asset	&$asset			the major or minor asset that we are linking to
	* @param string	$side_of_link	the side of the link we are on (major or minor)
	* @param string	&$link_type		the type of link we are creating
	* @param string	&$value			the value that is to be associated with the link
	* @param string	&$sort_order	the position in the links list that this link should take,
	* @param string	&$dependant		'0' / '1' on whether the this asset is dependant on the
	*								asset that will be linked by the new link
	* @param string	&$exclusive		'0' / '1' on whether the this asset is linked exclusivly
	*								to the asset that will be linked by the new link
	*
	* @return boolean
	* @access public
	*/
	public function prepareLink(Asset $asset, $side_of_link, &$link_type, &$value, &$sort_order, &$dependant, &$exclusive)
	{
		// if this is a workflow step then we need to make it a dependant link
		if ($side_of_link == 'major' && ($asset instanceof Workflow_Step) && $dependant != '1') {
			$dependant = '1';
			return TRUE;
		}

		return FALSE;

	}//end prepareLink()


}//end class

?>
