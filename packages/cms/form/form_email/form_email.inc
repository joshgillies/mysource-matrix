<?php

require_once  dirname(__FILE__).'/../form/form.inc';

/**
* Form_Email
*
* Purpose
*
*
* @author  Marc Mcintyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Form_Email extends Form
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Form_Email($assetid=0) 
	{
		$this->Form($assetid);
	}


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
		$linkid = parent::create($link);
		if (!$linkid) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}

		// give a name to this form asset
		$this->setAttrValue('name', 'Form Contents');

		// create a section for them to start off with
		$GLOBALS['SQ_SYSTEM']->am->includeAsset('form_section');
		$section = new Form_Section();
		$copy_link = Array('asset' => &$this, 'link_type' => SQ_LINK_TYPE_2, 'dependant' => 1, 'exclusive' => 1);

		$section->setAttrValue('name', 'Section 1');
		if (!$section->create($copy_link)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$this->id = 0;
			return false;
		}
		

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return $linkid;

	} // end create()


	/**
	* Creates a link between this asset and the passed asset, of a certain type
	* and returns the linkid of the created link
	*
	* @return int
	* @access public
	* @see Asset::creatLink();
	*/
	function createLink(&$minor, $link_type, $value='', $sort_order=-1, $dependant='0', $exclusive='0')
	{
		// if this is a bodycopy table then we need to make it a dependant link
		if (is_a($minor, 'bodycopy')) {
			$dependant = '1';
		}

		return parent::createLink($minor, $link_type, $value, $sort_order, $dependant, $exclusive);

	}// end createLink()


	/**
	* Returns TRUE if we can delete the passed link, or a string with the error msg
	*
	* @param int	$linkid	the link id of the link to remove
	*
	* @return mixed TRUE or error msg string
	* @access public
	*/
	function canDeleteLink($linkid)
	{
		// the only reason why we would object to this link being
		// deleted is if it is the bodycopy linked to us
		$bodycopy_link = $this->getLinks(SQ_LINK_TYPE_2, 'bodycopy', true, 'major', '', '1', '1');
		if ($bodycopy_link[0]['linkid'] == $linkid && !$GLOBALS['SQ_PURGING_TRASH']) return 'You cant delete the link between the bodycopy and the standard page';

		return parent::canDeleteLink($linkid);
	}//end canDeleteLink()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$page_links = parent::_getAllowedLinks();
		return $page_links;

	}//end _getAllowedLinks()


	/**
	* Prints out the backend interface for the asset
	*
	* @param object Backend_Outputter $o
	*
	* @access public
	*/
	function paintBackend(&$o)
	{
		parent::paintBackend($o);

	}// end paintBackend()


	/**
	* Called by the design to print the body of this asset
	* it's up to the kids to override
	*
	* @access public
	*/
	function printBody()
	{
		parent::printBody();
			
	}// end printBody()

	/**
	* Returns name of the asset
	*
	* @param bool	$short_name	whether or not we are after the shortname or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');
	}//end _getName();


	function finishForm()
	{
		parent::finishForm();
		$this->mailRecipients();
		$this->mailReceipt();
	}


	/**
	* returns a summary about the answers in the form submission
	*
	* @param boolean	$html	return html formatted or not 
	*
	* @access public
	* @return string $summary
	*/
	function getSummary($html = false)
	{
		$answers  = $this->active_submission->getAnswers();
		$title = 'Form Answers';
		$summary  = ($html) ? '<B>'.$title.'</B><BR />' : $title."\n\n";
		$summary .= ($html) ? "<table>" : '';
		
		foreach ($answers as $answer) {
			$summary .= ($html) ? '<tr><td>'.$answer['name'] .'</td>' : $answer['name']."\t\t";
			$summary .= ($html) ? '<td>'.$answer['answer'] .'</td></tr>' : $answer['answer']."\n";
		}
		
		$summary .= ($html) ? '</table>' : '';
		return $summary;
	}


	/**
	* returns form information (site name, site id, page name, page id, url, submission date)
	*
	* @param boolean	$html	return html formatted or not 
	*
	* @access public
	* @return string $information
	*/
	function getFormInfo($html = false)
	{
		$info = parent::getFormInfo();
		$title = 'Form Information'; 
		$information  = ($html) ? '<B>'.$title.'</B><BR />' : $title."\n\n";
		$information .= ($html) ? "<table>" : '';
		
		foreach ($info as $name => $value) {
			$information .= ($html) ? '<tr><td>'.ucwords(str_replace('_', ' ', $name)).'</td>' : $name."\t\t";
			$information .= ($html) ? '<td>'.$value.'</td></tr>' : $value."\n";
		}
		$information .= ($html) ? "</table>" : '';
		return $information;
	}

	function mailReceipt()
	{

	}


	/**
	* sends mail to recipients set in the backend
	* 
	* @access public
	* @return boolean
	*/
	function mailRecipients()
	{
		// return false if there is no-one to send to
		if (!$this->attr('recipient_emails')) return false;

		$addresses  = split("\r\n", $this->attr('recipient_emails'));
		
		$allow_html = ($this->attr('recipient_allow_html')) ? true : false; 
		$terminator = $allow_html ? '<BR />' : "\n"; // i'll be back!

		// if this email is in html, add some nice html tags
		$body = ($allow_html) ? "<html>\n\t<head><title></title></head>\n\t</body>" : '';
		$body .= $this->attr('recipient_body').$terminator.$terminator;

		// get the form info if they want it
		if ($this->attr('recipient_send_info')) {
			$body .= $this->getFormInfo($allow_html).$terminator;
		}

		// check to see if we want to send the answers to the recipient
		if ($this->attr('recipient_send_answers')) {
			$body .= $this->getSummary($allow_html).$terminator.$terminator;
		}

		// end the html
		$body .= ($allow_html) ? "</body></table>" : '';

		// send a html header if we need to
		$extras = ($allow_html) ? "\nContent-Type: text/html; charset=iso-8859-1" : '';

		$subject = ($this->attr('recipient_subject')) ? $this->attr('recipient_subject') : 'Form Submission';
	
		// special information address
		$si_address = $this->getSpecialInformation('email'); 
		if ($si_address === '') $si_address = false; 

		// spam... baby, spam!
		foreach ($addresses as $address) {
			// make sure these are valid emails, otherwise, just strip them out
			if (!$this->isValidEmail(trim($address))) continue;
			
			$from_address = ($si_address) ? $si_address : trim($address);
			mail(trim($address), $subject, $body, 'From: '.$from_address.$extras);
		}
		
		return true;
	}


	/**
	* returns if a email is valid or not
	*
	* @param string $email the address to check 
	*
	* @access public
	* @return boolean
	*/
	function isValidEmail($email)
	{
		return preg_match('/^([a-zA-Z0-9_-])+(\.[a-zA-Z0-9_-]+)*@([a-zA-Z0-9])+(\.[a-zA-Z0-9_-]+)+$/', $email);
	}


}//end class
?>
