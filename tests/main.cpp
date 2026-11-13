//==============================================================================
// Ponto de entrada dos testes do djdeck.
//
// PORQUE ESTE ARQUIVO EXISTE
//
// O target de testes linka juce_gui_basics (CrossfaderComponent e um
// Component). Isso traz o MessageManager e a GUI do JUCE para o processo.
//
// O crash (SIGSEGV / exit 139) acontecia SEM NENHUMA SAIDA — nem "--list-tests"
// produzia uma linha. Isso indica que o processo morre ANTES do main() do
// Catch2, ou seja, na inicializacao estatica.
//
// A causa: um objeto global de inicializacao do JUCE (MessageManagerBootstrap)
// roda o ctor de inicializacao do JUCE ANTES do main. No Windows, isso cria a
// message thread e a GUI fora do contexto correto da thread de mensagens, e o
// processo morre na teardown estatica.
//
// A SOLUCAO: inicializar o JUCE DENTRO do main(), depois que o runtime da
// C++ ja subiu e antes de rodar os testes. Isso e exatamente o que a
// documentacao do JUCE recomenda para apps de console:
//
//   "This class is particularly handy to use at the beginning of a console
//    app's main() function, because it'll take care of shutting down whenever
//    you return from the main() call."
//
// Por isso o target linka Catch2::Catch2 (sem main) e nao Catch2WithMain.
//
// Os marcadores fprintf para stderr sao deliberados: stderr nao e bufferizado
// no Windows, entao eles aparecem mesmo se o processo morrer em seguida. Eles
// delimitam as fases do startup para localizar regressoes futuras.
//==============================================================================

#define CATCH_CONFIG_RUNNER
#include <catch2/catch_session.hpp>

#include <juce_events/juce_events.h>

#include <cstdio>

int main (int argc, char* argv[])
{
    std::fprintf (stderr, "[djdeck_tests] fase 1: antes de initialiseJuce_GUI\n");
    std::fflush (stderr);

    // Cria e destroi a message thread / GUI do JUCE dentro do main.
    juce::ScopedJuceInitialiser_GUI juceInitialiser;

    std::fprintf (stderr, "[djdeck_tests] fase 2: JUCE inicializado, rodando Catch2\n");
    std::fflush (stderr);

    const int result = Catch::Session().run (argc, argv);

    std::fprintf (stderr, "[djdeck_tests] fase 3: Catch2 finished, result=%d\n", result);
    std::fflush (stderr);

    return result;
}