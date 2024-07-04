Contributing
############

Anyone is welcome to contribute to this project or create their own variant of it. I would appreciate a PR if your feature would be of benefit other users. 

In order to keep the source code in good condition I use `pre-commit <https://pre-commit.com/>`_ to validate and format the code using their standards for C++/C. 

.. note::  

  If you are using Windows as a base platform I would suggest that you install pre-commit under wsl (Windows Subsystem for Windows) and run it from there, I have found 
  that this approach works fine. 


The following command will run pre-commit on all the source files. Assuming you are in the project directory.

.. code-block:: 

  pre-commit run --files src/*


Design goals
------------

My goals with this software has been the following:

* Create an open software for the excellent iSpindle hardware platform that is open and available for anyone. 
* Add user requested features that have not made it into the iSpindle project. 
* Focus on long battery life and stability.
* Explore new technologies and create the next generation gravity monitoring for home brewers. 

I will do my best to assist users and respond to new features, pr and suggestions. But keep in mind I'm doing this on my spare time. 

Regards, Magnus
