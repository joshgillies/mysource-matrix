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
* $Id: report.inc,v 1.10 2007/12/03 05:18:55 lwright Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/page/page.inc';

/**
* Report
*
* Purpose
*
*
* @author  Greg Sherwoood <greg@squiz.net>
* @version $Revision: 1.10 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Report extends Page
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function __construct($assetid=0)
	{
		parent::__construct($assetid);

	}//end constructor


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the shortname or the full name
	*
	* @return string
	* @access private
	* @see Asset::_getName()
	*/
	protected function _getName($short_name=FALSE)
	{
		return $this->attr('name');

	}//end _getName()


	/**
	* Returns a list of lock types available for this asset type
	*
	* @return array
	* @access public
	*/
	public function lockTypes()
	{
		$lock_types = parent::lockTypes();
		$lock_types['content'] = ($lock_types['attributes'] | $lock_types['links']);
		return $lock_types;

	}//end lockTypes()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @return array
	* @access private
	* @see Asset::_getAllowLinks()
	*/
	public function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		$page_links[SQ_LINK_NOTICE]['asset'] = Array('card' => 'M', 'exclusive' => FALSE);
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints the report
	*
	* note: edit.css might conflict with user defined css style in the design
	*
	* @return void
	* @access public
	*/
	public function printBody()
	{
		?>
		<script type="text/javascript" src="<?php echo sq_web_path('lib');?>/js/general.js"></script>
		<link rel="stylesheet" type="text/css" href="<?php echo sq_web_path('lib');?>/web/css/edit.css" />
		<?php
		require_once SQ_INCLUDE_PATH.'/limbo_outputter.inc';
		$o = new Limbo_Outputter();
		$o->openSection();
			$efns = $this->getEditFns();
			$efns->paintReport($this, $o, $this->getPrefix());
		$o->closeSection();
		$o->paintSections();

	}//end printBody()


}//end class

?>