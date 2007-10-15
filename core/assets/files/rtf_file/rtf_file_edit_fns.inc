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
* $Id: rtf_file_edit_fns.inc,v 1.1 2007/10/15 05:51:27 bshkara Exp $
*/


require_once SQ_CORE_PACKAGE_PATH.'/files/file/file_edit_fns.inc';

/**
* Rtf_File_Edit_Fns
*
* Purpose
*
*
* @author  Basil Shkara <bshkara@squiz.net>
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Rtf_File_Edit_Fns extends File_Edit_Fns
{


    /**
    * Constructor
    *
    */
    function Rtf_File_Edit_Fns()
    {
        $this->File_Edit_Fns();

    }//end constructor


}//end class
?>
