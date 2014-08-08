<?php
/**
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600													   |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
*
* $Id: web_folder_web_services.inc,v 1.7 2012/08/30 00:59:46 ewang Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/web_folders/web_folder/web_folder.inc';

/**
* Web Folder Web Services
*
* Purpose
*
*
* @author  Huan Nguyen <hnguyen@squiz.net>
* @version $Revision: 1.7 $
* @package MySource_Matrix_Packages
* @subpackage web_services
*/
class Web_Folder_Web_Services extends Web_Folder
{


    /**
    * Constructor
    *
    * @param int    $assetid    the asset id to be loaded
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
				SQ_LINK_TYPE_1 => Array(
									'soap_server'	=> Array('card' => 'M', 'exclusive' => FALSE),
									'js_api'		=> Array('card' => 'M', 'exclusive' => FALSE),
								  ),
				SQ_LINK_TYPE_2 => Array(
									'soap_server'	=> Array('card' => 'M', 'exclusive' => FALSE),
									'js_api'		=> Array('card' => 'M', 'exclusive' => FALSE),
								  ),
				SQ_LINK_NOTICE => Array('design' => Array('card' => 'M', 'exclusive' => FALSE)),
			   );

    }//end _getAllowedLinks()


    public function getDesignParseFile()
    {
    	$parse_file = '<html>
	<head></head>
<body style="margin-left:0px;margin-right:0px;margin-top:0px;">
<div style="height:100%;width:100%;border: 1px thin;">
   <div style="width:100%;height:70px; background-color:#003366;padding-top:30px;padding-left:20px;">
     <span style="color:#FFFFFF;font-weight:bold;font-family:verdana;font-size:20px;">
        MATRIX Web Services
     </span>
   </div>

<MySource_AREA id_name="page_lineage" design_area="asset_lineage">

<MySource_SET name="levels_to_print" value="0" />
<MySource_SET name="prefix_with_home_link" value="false" />

<MySource_SET name="suffix_with_current_link" value="true" />
<MySource_SET name="prefix_with_divider" value="true" />
<MySource_SET name="suffix_with_divider" value="false" />

<MySource_DIVIDER>&gt;&nbsp;</MySource_DIVIDER>

</mySource_Area>

<div style="padding-left:18px;padding-top:18px;">
<span style="color:black;font-weight:bold;font-size:14px;font-family:verdana;">
   Available Operations
</span>
<br /><br />
<MySource_AREA id_name="page_body" design_area="body" />
</div>
</div>
</body></html>';

    	return $parse_file;

    }//end getDesignParseFile()

	function canDelete()
	{
		return TRUE;
	}
}//end class
?>
