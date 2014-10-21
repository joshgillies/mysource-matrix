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
* $Id: content_type_raw_html_prefs.inc,v 1.15 2012/08/30 00:58:07 ewang Exp $
*
*/


require_once SQ_LIB_PATH.'/config/prefs.inc';
require_once SQ_ATTRIBUTES_PATH.'/css_dimension/css_dimension.inc';

/**
* Content_Type_Raw_HTML_Prefs
*
* Purpose
*
*    Looks after the creation the Raw HTML preferences
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.15 $
* @package MySource_Matrix
*/
class Content_Type_Raw_HTML_Prefs extends Prefs
{

	var $pref_vars = Array (
						'SQ_RAW_HTML_SIZE'	=> Array(
												'default'		=> Array('width' => '70em', 'height' => '20em'),
												'protected'		=> FALSE,
											   ),
					 );
	var $css_dimension_width;
	var $css_dimension_height;


	/**
	* Constructor
	*
	*/
	function Content_Type_Raw_HTML_Prefs($pref_file='')
	{
		$this->Prefs($pref_file);
		$this->css_dimension_width = new Asset_Attribute_CSS_Dimension();
		$this->css_dimension_height = new Asset_Attribute_CSS_Dimension();

		$this->pref_vars['SQ_RAW_HTML_SIZE']['name'] = translate('Raw HTML Editor Size');

	}//end constructor


	/**
	* Paints the backend interface to edit preferences
	*
	* @param object		&$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to paint
	*
	* @return boolean
	* @access public
	*/
	function paintBackend(&$o, $have_lock, $pref=NULL)
	{
	    $is_root   = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin  = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
        $have_lock = ($have_lock && $is_admin);

		$this->paintField($o, $have_lock, $pref, 'SQ_RAW_HTML_SIZE', function($html_size) use ($have_lock) {
            ?>
            <table class="sq-backend-table" style="width: 350px;">
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Width') ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if ($have_lock) {
                        $this->css_dimension_width->paint('width', $have_lock, $html_size['width']);
                    } else {
                        echo $html_size['width'];
                    }
                    ?>
                    </td>
                </tr>
                <tr>
                    <td class="sq-backend-table-header" nowrap="nowrap"><?php echo translate('Height') ?></td>

                    <td class="sq-backend-table-cell" style="width: 100%;">
                    <?php
                    if ($have_lock) {
                        $this->css_dimension_height->paint('height', $have_lock, $html_size['height']);
                    } else {
                        echo $html_size['height'];
                    }
                    ?>
                    </td>
                </tr>
            </table>
            <?php
		});

		return TRUE;

	}//end paintBackend()


	/**
	* Processes the backend interface to edit preferences
	*
	* @param object		$o			reference to the backend outputter
	* @param boolean	$have_lock	do we have the lock to edit these preferences or not
	* @param string		$pref		a particular preference to process
	*
	* @return boolean
	* @access public
	*/
	function processBackend($o, $have_lock, $pref=NULL)
	{
		$is_root  = $GLOBALS['SQ_SYSTEM']->userRoot();
		$is_admin = ($is_root || $GLOBALS['SQ_SYSTEM']->userSystemAdmin());
		if (!$is_admin) return FALSE;

		if (isset($_POST['width']) && isset($_POST['height'])) {
			$this->css_dimension_width->process('width');
			$this->css_dimension_height->process('height');
			$this->pref_vars['SQ_RAW_HTML_SIZE']['default']['width'] = $this->css_dimension_width->getContent();
			$this->pref_vars['SQ_RAW_HTML_SIZE']['default']['height'] = $this->css_dimension_height->getContent();
			return TRUE;
		}
		return FALSE;

	}//end processBackend()


}//end class

?>
