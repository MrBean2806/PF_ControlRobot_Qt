## UI para el control manual del Robot Omnidireccional
El propósito de este programa de Qt es permitir el control manual del robot omnidireccional y la visualización de las variables medidas por el mismo.

El programa se comunica directamente con el controlador de bajo nivel por puerto serie, sin necesidad de una computadora de navegación.
Para conectarse a la PC, se debe conocer el vendor id y el product id del conversor usb y agregarlo a los miembros de la clase MainWindow. Hay una porcion de codigo comentada que permite obtener estos valores.

Hay funcionalidades que no se terminaron de implementar, como el uso de un CRC para comprobar la trama o mostrar el status de los perifericos.

![image](https://github.com/MrBean2806/PF_ControlRobot_Qt/assets/39303764/495ce8e2-fea4-4eea-9563-db8096763f5b)
