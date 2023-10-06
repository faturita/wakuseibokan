# Escenario 111

La bala sale con una velocidad inicial de 600 m/s.  El ángulo de tiro es directamente el valor en grados en “pitch” con 90 grados positivo apuntando al zenith.

Sin embargo la bala sale de la boca de la torreta que son 2 metros arriba del centro de masa del tanque y unos 40 metros por delante es cuando se dibuja por primera vez (esto se hace para que no toque al tanque de donde sale).

Además hay drag del aire por lo que eso reduce la velocidad punto a punto.

La estrategia de tratar de pegarle con tiro oblicuo es genial, pero hay que tolerar algo de margen de error por esas variaciones.

El tanque a máxima velocidad va a unos 28 m/s.

Finalmente El Paso de simulación corresponde a 20 ticks.   Por lo que si el tanque va a 28 m/s quiere decir que en 20 ticks recorre 28 metros.

## Conectividad

El server usa ese *telemetry.endpoints.ini* para mandarle el paquete de telemetría x UDP a todos los clientes que estén ahí listados en cada uno de las IPs, puertos tal como se configura en ese archivo.
Esa es la info de telemetría, la info de los tanques.  Le mando todo a todos.

El server también se levanta en dos puestos fijos el 4601 y 4602 que usa para recibir los comandos del primer y el segundo tanque.

Después cada cliente en Python usa dos sockets.  El primero *server_socket* es el que usa para recibir la información de telemetría que le manda el servidor.   Eso es lo que permite procesar en el loop todos los mensajes momento a momento que manda el servidor con la info de la ubicación punto a punto de los tanques.

El segundo el *control_socket* que es el que los clientes en Python usan para mandarle al servidor los comandos.

# Escenario 119

Este escenario muestra cómo es el tiro oblicuo del disparo de un tanque donde está fijo el valor de velocidad inicial a 100 m/s y se le quitó el drag.
La distancia entre los tanques es de 1000 m.
